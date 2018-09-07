#include <iostream>
#include <sstream>
#include "TFile.h"
#include "TTree.h"
#include "TKey.h"
#include "TRandom3.h"
#include "TMath.h"

#include "library_access_gutted.h"

using namespace std;

LibraryAccess::LibraryAccess()
	: table_(std::vector<std::vector<float> >()),
	reflected_table_(std::vector<std::vector<float> >()),
	reflT_table_(std::vector<std::vector<float> >())
{

}



void LibraryAccess::LoadLibraryFromFile(std::string libraryfile, bool reflected, bool reflT0)
{
	cout << "Reading photon library from input file: " << libraryfile.c_str()<<endl;

	TFile *f = nullptr;
	TTree *tt = nullptr;

	try
	{
		f  =  TFile::Open(libraryfile.c_str());
		tt =  (TTree*)f->Get("PhotonLibraryData");

		if (!tt) {

			TKey *key = f->FindKeyAny("PhotonLibraryData");
			if (key)
				tt = (TTree*)key->ReadObj();
			else {
				cout << "PhotonLibraryData not found in file" <<libraryfile;
			}
		}
	}
	catch(...)
	{
		cout << "Error in ttree load, reading photon library: " << libraryfile.c_str()<<endl;
	}

	int voxel;
	int opChannel;
	float visibility;
	float reflVisibility;
	float reflT;
	int maxvoxel = 75*125*300;
	int maxopChannel = tt->GetMaximum("OpChannel")+2;

	cout << "Photon lookup table size : " <<  maxvoxel << " voxels,  " << maxopChannel <<" channels " << endl;

	table_.resize(maxvoxel, std::vector<float>(maxopChannel, 0));
	reflected_table_.resize(maxvoxel, std::vector<float>(maxopChannel, 0));
	reflT_table_.resize(maxvoxel, std::vector<float>(maxopChannel, 0));


	tt->SetBranchAddress("Voxel",      &voxel);
	tt->SetBranchAddress("OpChannel",  &opChannel);
	tt->SetBranchAddress("Visibility", &visibility);
	if(reflected) {tt->SetBranchAddress("ReflVisibility", &reflVisibility); }
	if(reflT0) {tt->SetBranchAddress("ReflTfirst", &reflT); }

	size_t nentries = tt->GetEntries();
    int j = 1;
	for(size_t i=0; i!=nentries; ++i)
	{
	    if ( (i % (nentries/10) == 0) && (i != 0) ) {
            cout << Form("%i0%% Completed...\n", j);
            j++;
        }
	    
		tt->GetEntry(i);
		if((voxel<0)||(voxel>= maxvoxel)||(opChannel<0)||(opChannel>= maxopChannel))
		{}
		else
		{
			table_.at(voxel).at(opChannel) = visibility;
			if(reflected) {reflected_table_.at(voxel).at(opChannel) = reflVisibility; }
			else{reflected_table_.at(voxel).at(opChannel) = 0; }
			if(reflT0) {reflT_table_.at(voxel).at(opChannel) = reflT; }
			else{reflT_table_.at(voxel).at(opChannel) = 0; }
		}
	}

	try
	{
		f->Close();
	}
	catch(...)
	{
		cout << "Error in closing file : " << libraryfile.c_str()<<endl;
	}
}

const float* LibraryAccess::GetReflT0(size_t voxel, int no_pmt)
{
	return &reflT_table_.at(voxel).at(no_pmt);
}

const float* LibraryAccess::GetReflCounts(size_t voxel, int no_pmt, bool reflected)
{
	if(reflected) {return &reflected_table_.at(voxel).at(no_pmt); }
	else{return 0; }
}

const float* LibraryAccess::GetCounts(size_t voxel, int no_pmt)
{
	return &table_.at(voxel).at(no_pmt);
}

const float* LibraryAccess::GetLibraryEntries(int voxID, bool reflected, int no_pmt)
{
	if(!reflected)
		return GetCounts(voxID, no_pmt);
	else
		return GetReflCounts(voxID, no_pmt, reflected);
}
