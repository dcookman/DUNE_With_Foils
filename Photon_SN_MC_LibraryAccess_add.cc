// Photon_SN_MC1.cc
// Daniel Cookman 2018/06

//INCLUDE STATEMENTS
#include<vector>
#include<math.h>
#include<string>

#include "library_access_gutted.h"
//GLOBAL CONSTANTS
const vector<Int_t> voxlims = {75, 125, 300};
const vector<Float_t> DetMin = {0., -658.099, -302.946}; //cm
const vector<Float_t> DetMax = {379.662, 699.599, 1443.53}; //cm
const vector<Float_t> EffMax = {379.662, 699.599*121/125, 1443.53}; //cm
const vector<Float_t> voxlens = {
    (DetMax[0]-DetMin[0])/voxlims[0],
    (DetMax[1]-DetMin[1])/voxlims[1],
    (DetMax[2]-DetMin[2])/voxlims[2]
};
const double NUMPHOTONS = 500000.; // Number of photons emitted per voxel in the original MC
const double PHOTPERENERGY = 24000.; // Number of photons emitted per MeV of energy deposited

//FUNCTIONS
vector<int> GetVoxelPos(int V) {
    // From the voxel number, produces the position of the voxel in terms of voxel units.
    vector<int> VPos = {V % voxlims[0], (V / voxlims[0]) %voxlims[1], (V / (voxlims[0]*voxlims[1]))};
    return VPos;
}
int GetVoxelNum(vector<int> VPos) {
    // Takes in a Voxel position vector and outputs its number.
    return ( VPos[2]*voxlims[0]*voxlims[1] + VPos[1]*voxlims[0] + VPos[0] );
}
vector<int> PosToVPos(vector<Float_t> Pos) {
    // Converts a position vector in real space to the associated voxel number
    // Do this by first converting to voxel space
    vector<int> VPos;
    for (int i=0; i<3; i++) {
        VPos.push_back(int(floor((Pos[i]-DetMin[i])/voxlens[i])));
    }
    return VPos;
}
int PosToVNum(vector<Float_t> Pos) {
    // Converts a position vector in real space to the associated voxel number
    // Do this by first converting to voxel space
    return GetVoxelNum(PosToVPos(Pos));
}

int GetRandVoxInRange(vector<Float_t> PosMin, vector<Float_t> PosMax) {
    // Within the 3D range defined by the input arguments, find a random point
    // in space and then output the associated voxel number.
    vector<Float_t> RandPos;
    gRandom->SetSeed();
    for (int i=0; i<3; i++) {
        RandPos.push_back(gRandom->Uniform(PosMin[i],PosMax[i]));
    }
    //cout << Form("Position: {%g,%g,%g}\n",RandPos[0],RandPos[1],RandPos[2]);
    return PosToVNum(RandPos);
}

Float_t GetEnergy() {
    return 100.; //MeV
}

// ***** MAIN *****
int Photon_SN_MC(int n, int LY_min, int LY_max, bool is_bars, Float_t catcov,  const char *outfilename = "Photon_SN_MC1.root", vector<Float_t> PosMin = DetMin, vector<Float_t> PosMax = EffMax, Float_t vuvfrac = 1.) {
    /*
    Photon_SN_MC: main function that randomly chooses SN energy depositions within the detector, outputting a series of histograms
    corresponding to the amount of light detected.
    
    INPUT ARGUMENTS:
    - n:            number of energy depositions to create
    - LY_min/max:   minimum and maximum bin values for the hists created
    - is_bars:      true for Light guide bars, false for fake ARAPUCA detectors
    - catcov:       proportion of cathode covered by TPB
    - outfilename:  filename for the hists to be saved to
    - PosMin/Max:   defines allowed region for depositions to be created within
    - vuvfrac:      used for ARAPUCA setup ONLY; proportion of SiPMs with TPB on them
    */
    
    // Load the photon library and build the index for random access
    Float_t QE;
    Float_t visfrac = 1.; // - vuvfrac;
    if (is_bars) {
        QE = 0.0042*0.7;
    }
    else {
       QE = 0.025*0.7/0.46; // 0.46 due to bar attenuation factor baked into library
    }
    
    cout << "LOADING IN LIBRARY...\n";
    LibraryAccess lar_light;
    lar_light.LoadLibraryFromFile("dune_withfoils_lib_cleaned.root",true,false); //NEW!!!!
    cout << "LIBRARY LOADED!\n";
    int    Voxel;
    int    OpChannel;
    Float_t Visibility;
    Float_t ReflVisibility;
    
    TFile *fout = new TFile(outfilename,"RECREATE");
    // Create histogram arrays
    vector<TH1F*> vishista;
    vector<TH1F*> reflhista;
    vector<TH1F*> bothhista;
    for (int j=0; j<voxlims[0]; j++) {
        TH1F *vh = new TH1F(Form("vh%i",j), Form("Visible Only %i",j),  3000,LY_min,LY_max);
        TH1F *rh = new TH1F(Form("rh%i",j), Form("Reflected Only %i",j),3000,LY_min,LY_max);
        TH1F *bh = new TH1F(Form("bh%i",j), Form("Both %i",j),          3000,LY_min,LY_max);
        vishista.push_back(vh);
        reflhista.push_back(rh);
        bothhista.push_back(bh);
    }
    
    
    //Loop this N times, building up a data set of LY stored in an array of histograms:
    gRandom->SetSeed();
    int randVox;
    Float_t E;
    Float_t v; // Poisson-fluctuated number of hits of the original MC simulation for a given OpC
    Float_t r;
    Float_t vnumHits;
    Float_t rnumHits;
    Float_t bnumHits;
    vector<int> VPos;
    int x;
    int k = 1;
    for(int j=0; j<n; j++) {
        if ( (j % (n/10) == 0) && (j != 0) ) {
            cout << Form("%i0%% Completed...\n", k);
            k++;
        }
        //Choose uniformly a random point in detector, possibly within some user-defined subsection
        randVox = GetRandVoxInRange(PosMin, PosMax);
        VPos = GetVoxelPos(randVox);
        x = VPos[0];
        //Get an energy deposition (10MeV for now)
        E = GetEnergy();
        
        //cout << Form("====DATA POINT %i====\n",j);
        //cout << Form("randVox:\t%i\tVPos:\t{%i,%i,%i}\tE:\t%g\n",randVox,VPos[0],VPos[1],VPos[2],E); 
        //Calculate expected number of hits on the detector with/out the reflector foil, Poisson fluctuated
        vnumHits = 0.;
        rnumHits = 0.;
        bnumHits = 0.;
        //Iterate over OpChannels:
        for (int OpC=0 ;OpC<120; OpC++) {
            //From chosen randVox along with OpC find associated visibilities from library, Poisson fluctuating them
            Visibility = *(lar_light.GetLibraryEntries(randVox,false,OpC));
            ReflVisibility = *(lar_light.GetLibraryEntries(randVox,true,OpC));
            
            v = vuvfrac*QE*(gRandom->Poisson(PHOTPERENERGY*Visibility*E))/(2.*E);
            r = visfrac*catcov*QE*(gRandom->Poisson(PHOTPERENERGY*ReflVisibility*E))/E;
            
            vnumHits += v;
            rnumHits += r;
            bnumHits += (v+r);
            //cout << Form("OpC:\t%i\tVis:\t%g\tRVis:\t%g\tv:\t%g\tr:\t%g\n",OpC,Visibility,ReflVisibility,v,r);
        }
        //cout << Form("vnumHits:\t%g\tbnumHits:\t%g\n\n",vnumHits,bnumHits);
        //Fill hit values to histograms
        vishista[x] ->Fill(vnumHits);
        reflhista[x]->Fill(rnumHits);
        bothhista[x]->Fill(bnumHits);
    }
    //Save data for analysis in another macro
    fout->Write();
    return 0;
    //cout << Form("randVox:\t%i\nOpC:\t%i\nE:\t%g\nVoxel:\t%i\nOpChannel:\t%i\nVisibility:\t%g\nReflVisibility:\t%g\n",randVox,OpC,E,Voxel,OpChannel,Visibility,ReflVisibility);
}

int PoissonDistTest(int n, const char *outfilename = "Photon_SN_MC1.root", vector<Float_t> PosMin = DetMin, vector<Float_t> PosMax = EffMax) {
    // Load the photon library and build the index for random access
    cout << "LOADING IN LIBRARY...\n";
    LibraryAccess lar_light;
    lar_light.LoadLibraryFromFile("dune_withfoils_lib_cleaned.root",true,false); //NEW!!!!
    cout << "LIBRARY LOADED!\n";
    int    Voxel;
    int    OpChannel;
    Float_t Visibility;
    Float_t ReflVisibility;
    
    TFile *fout = new TFile(outfilename,"RECREATE");
    // Create histogram arrays
    TH1F * vishista  = new TH1F("vishist", "Visible Only", 1000,0,3000);
    TH1F * bothhista = new TH1F("bothhist", "Both",        1000,0,3000);

    //Loop this N times, building up a data set of LY stored in an array of histograms:
    gRandom->SetSeed();
    int randVox = GetRandVoxInRange(PosMin, PosMax);
    Float_t E = GetEnergy();
    Float_t v; // Poisson-fluctuated number of hits of the original MC simulation for a given OpC
    Float_t r;
    Float_t vnumHits;
    Float_t bnumHits;
    vector<int> VPos = GetVoxelPos(randVox);
    int x = VPos[0];
    int k = 0;
    cout << Form("randVox:\t%i\tVPos:\t{%i,%i,%i}\tE:\t%g\n",randVox,VPos[0],VPos[1],VPos[2],E); 
    for(int j=0; j<n; j++) {
        if ( (j % (n/10) == 0) && (j != 0) ) {
            cout << Form("%i0%% Completed...\n", k);
            k++;
        }

        //Calculate expected number of hits on the detector with/out the reflector foil, Poisson fluctuated
        vnumHits = 0.;
        bnumHits = 0.;
        //Iterate over OpChannels:
        for (int OpC=0 ;OpC<120; OpC++) {
            //From chosen randVox along with OpC find associated visibilities from library, Poisson fluctuating them
            Visibility = *(lar_light.GetLibraryEntries(randVox,false,OpC));
            ReflVisibility = *(lar_light.GetLibraryEntries(randVox,true,OpC));
            v = (gRandom->Poisson(NUMPHOTONS*Visibility))/2.;
            r = gRandom->Poisson(NUMPHOTONS*ReflVisibility);
            vnumHits += v*(PHOTPERENERGY/NUMPHOTONS);
            bnumHits += (v+r)*(PHOTPERENERGY/NUMPHOTONS);
            //cout << Form("OpC:\t%i\tVis:\t%g\tRVis:\t%g\tv:\t%g\tr:\t%g\n",OpC,Visibility,ReflVisibility,v,r);
        }
        //cout << Form("vnumHits:\t%g\tbnumHits:\t%g\n\n",vnumHits,bnumHits);
        //Fill hit values to histograms
        vishista ->Fill(vnumHits);
        bothhista->Fill(bnumHits);
    }
    //Save data for analysis in another macro
    fout->Write();
    return 0;
    //cout << Form("randVox:\t%i\nOpC:\t%i\nE:\t%g\nVoxel:\t%i\nOpChannel:\t%i\nVisibility:\t%g\nReflVisibility:\t%g\n",randVox,OpC,E,Voxel,OpChannel,Visibility,ReflVisibility);
}

void randPosTest(int n, vector<Float_t> PosMin = DetMin, vector<Float_t> PosMax = EffMax) {
    // Test the ability of GetRandVoxInRange to work
    gRandom->SetSeed();
    cout << PosMin[0] << endl;
    for (int i=0; i<n; i++) {
        int randVox = GetRandVoxInRange(PosMin, PosMax);
        cout << randVox << endl;
    }
}
