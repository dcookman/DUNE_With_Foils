// visibilty_plane_plotter:
// For a given optical channel, coordinate direction and voxel slice number
// create a 2D colour plot visualising the visibility of this set of voxels.
//
// Daniel Cookman 2018/06

// include statements?
#include<vector>


const int voxlimx = 75;
const int voxlimy = 125;
const int voxlimz = 300;



vector<int> GetVoxelPos(int V) {
    // From the voxel number, produces the position of the voxel in terms of voxel units.
    // Assumes we number voxels by x, then y, then z!
    vector<int> VPos = {V % voxlimx, (V / voxlimx) %voxlimy, (V / (voxlimx*voxlimy))};
    return VPos;
}

int GetVoxelNum(vector<int> VPos) {
    // Performs inverse of GetVoxelPos(), taking in a Voxel position vector and outputting its number.
    return ( VPos[2]*voxlimx*voxlimy + VPos[1]*voxlimx + VPos[0] );
}

void PrintVoxelList() {
    // WARNING: WILL SPAM ~3,000,000 LINES TO CONSOLE
    // ONLY DO IF REALLY STUCK AND DON'T VALUE SANITY
    for (int i = 0; i<(voxlimx*voxlimy*voxlimz); i++) {
        vector<int> VPos = GetVoxelPos(i);
        cout << Form("%i\t%i\t%i\n", VPos[0], VPos[1], VPos[2]);
    }
}

void visibility_plane_plotter(int OpC, int CoordNum, int SliceNum) {
    
    // Load the photon library
    TFile *fin = new TFile("dune_withfoils_lib_cleaned_ordered.root");
    
    //Load the PhotonLibraryData tree and attach variables to the neccessary branches
    TTree *pltree = (TTree*)fin->Get("PhotonLibraryData");
    int    Voxel;           pltree->SetBranchAddress("Voxel",          &Voxel);
    int    OpChannel;       pltree->SetBranchAddress("OpChannel",      &OpChannel);
    Float_t Visibility;     pltree->SetBranchAddress("Visibility",     &Visibility);
    Float_t ReflVisibility; pltree->SetBranchAddress("ReflVisibility", &ReflVisibility);
    
    // Create Canvas & Histogram Objects
    TCanvas *c1;
    TH2F *vishist;
    TH2F *reflhist;
    TH2F *bothhist;
    if (CoordNum == 0) {
        c1 = new TCanvas("c1", Form("Visibility of X Plane %i for Optical Channel %i", SliceNum, OpC),900,700);
        c1->Divide(3);
        vishist  = new TH2F("vishist" , "Visible Only",     voxlimz,0,voxlimz,voxlimy,0,voxlimy);
        reflhist = new TH2F("reflhist", "Reflected Only",   voxlimz,0,voxlimz,voxlimy,0,voxlimy);
        bothhist = new TH2F("bothhist", "Both",             voxlimz,0,voxlimz,voxlimy,0,voxlimy);
    }
    else if (CoordNum == 1) {
        c1 = new TCanvas("c1", Form("Visibility of Y Plane %i for Optical Channel %i", SliceNum, OpC),900,700);
        c1->Divide(3);
        vishist  = new TH2F("vishist" , "Visible Only",     voxlimx,0,voxlimx,voxlimz,0,voxlimz);
        reflhist = new TH2F("reflhist", "Reflected Only",   voxlimx,0,voxlimx,voxlimz,0,voxlimz);
        bothhist = new TH2F("bothhist", "Both",             voxlimx,0,voxlimx,voxlimz,0,voxlimz);
    }
    else if (CoordNum == 2) {
        c1 = new TCanvas("c1", Form("Visibility of Z Plane %i for Optical Channel %i", SliceNum, OpC),900,700);
        c1->Divide(3);
        vishist  = new TH2F("vishist" , "Visible Only",     voxlimx,0,voxlimx,voxlimy,0,voxlimy);
        reflhist = new TH2F("reflhist", "Reflected Only",   voxlimx,0,voxlimx,voxlimy,0,voxlimy);
        bothhist = new TH2F("bothhist", "Both",             voxlimx,0,voxlimx,voxlimy,0,voxlimy);
    }
    
    //int u = 0;
    const int TotNum = pltree->GetEntries();
    int i = 1;
    //cout << "TotNum:\t" << TotNum << endl;
    //cout << "V\tVoxel\tOpChan\tVPosX\tVPosY\tVPosZ\n";

    cout << "Plotting data:\n";
    for (int V = 0; V < TotNum; V++) {
        // Get data points from tree leaves
        pltree->GetEntry(V);
        
        if ( (V % (TotNum/10) == 0) && (V != 0) ) {
            cout << Form("%i0%% Completed...\n", i);
            i++;
        }
        
        // If data point corresponds to relevant OpChannel and Slice, plot on histogram
        if (OpChannel == OpC) {
            vector<int> VPos = GetVoxelPos(Voxel);
            
            if (VPos[CoordNum] == SliceNum) {
                //cout << V << "\t" << Voxel << "\t" << OpChannel << "\t" << VPos[0] << "\t" << VPos[1] << "\t" << VPos[2] << endl;
                //u++;
                if (CoordNum == 0) {
                    vishist->Fill(VPos[2], VPos[1], 0.5*Visibility);
                    reflhist->Fill(VPos[2], VPos[1], ReflVisibility);
                    bothhist->Fill(VPos[2], VPos[1], 0.5*Visibility +ReflVisibility);
                }
                else if (CoordNum == 1) {
                    vishist->Fill(VPos[0], VPos[2], 0.5*Visibility);
                    reflhist->Fill(VPos[0], VPos[2], ReflVisibility);
                    bothhist->Fill(VPos[0], VPos[2], 0.5*Visibility +ReflVisibility);
                }
                else if (CoordNum == 2) {
                    vishist->Fill(VPos[0], VPos[1], 0.5*Visibility);
                    reflhist->Fill(VPos[0], VPos[1], ReflVisibility);
                    bothhist->Fill(VPos[0], VPos[1], 0.5*Visibility +ReflVisibility);
                }
            }
        }
    }
    //cout << u << endl;  
    // Finally, Draw the data we have plotted to canvas!
    double maxval = bothhist->GetMaximum();
    //gStyle->SetOptStat(0);
    c1->cd(3);
    bothhist->SetMaximum(maxval);
    bothhist->Draw("colz");
    
    c1->cd(1);
    vishist->SetMaximum(maxval);
    vishist->Draw("col same");
    
    c1->cd(2);
    reflhist->SetMaximum(maxval);
    reflhist->Draw("col same");
    //pltree->Draw("Voxel:Visibility>>mynewhist",Form("OpChannel==%d",OpC));
}

void visibility_plane_plotter_special(int OpC, int CoordNum, int SliceNum) {
    
    // Load the photon library
    TFile *fin = new TFile("dune_withfoils_lib.root");
    
    //Load the PhotonLibraryData tree and attach variables to the neccessary branches
    TTree *pltree = (TTree*)fin->Get("pmtresponse/PhotonLibraryData");
    int    Voxel;           pltree->SetBranchAddress("Voxel",          &Voxel);
    int    OpChannel;       pltree->SetBranchAddress("OpChannel",      &OpChannel);
    Float_t Visibility;     pltree->SetBranchAddress("Visibility",     &Visibility);
    Float_t ReflVisibility; pltree->SetBranchAddress("ReflVisibility", &ReflVisibility);
    
    // Create Canvas & Histogram Objects
    TCanvas *c1;
    TH2F *vishist;
    TH2F *reflhist;
    TH2F *bothhist;
    if (CoordNum == 0) {
        c1 = new TCanvas("c1", Form("Visibility of X Plane %i for Optical Channel %i", SliceNum, OpC),900,700);
        c1->Divide(3);
        vishist  = new TH2F("vishist" , "Visible Only",     voxlimz,0,voxlimz,voxlimy,0,voxlimy);
        reflhist = new TH2F("reflhist", "Reflected Only",   voxlimz,0,voxlimz,voxlimy,0,voxlimy);
        bothhist = new TH2F("bothhist", "Both",             voxlimz,0,voxlimz,voxlimy,0,voxlimy);
    }
    else if (CoordNum == 1) {
        c1 = new TCanvas("c1", Form("Visibility of Y Plane %i for Optical Channel %i", SliceNum, OpC),900,700);
        c1->Divide(3);
        vishist  = new TH2F("vishist" , "Visible Only",     voxlimx,0,voxlimx,voxlimz,0,voxlimz);
        reflhist = new TH2F("reflhist", "Reflected Only",   voxlimx,0,voxlimx,voxlimz,0,voxlimz);
        bothhist = new TH2F("bothhist", "Both",             voxlimx,0,voxlimx,voxlimz,0,voxlimz);
    }
    else if (CoordNum == 2) {
        c1 = new TCanvas("c1", Form("Visibility of Z Plane %i for Optical Channel %i", SliceNum, OpC),900,700);
        c1->Divide(3);
        vishist  = new TH2F("vishist" , "Visible Only",     voxlimx,0,voxlimx,voxlimy,0,voxlimy);
        reflhist = new TH2F("reflhist", "Reflected Only",   voxlimx,0,voxlimx,voxlimy,0,voxlimy);
        bothhist = new TH2F("bothhist", "Both",             voxlimx,0,voxlimx,voxlimy,0,voxlimy);
    }

    const int TotNum = pltree->GetEntries();
    int i = 1;
    vector<int> VPos;
    // Iterate over all data points in tree to find relevant data
    cout << "Plotting data:\n";
    for (int V = 0; V < TotNum; V++) {
        // Get data points from tree leaves
        pltree->GetEntry(V);
        
        if ( (V % (TotNum/10) == 0) && (V != 0) ) {
            cout << Form("%i0%% Completed...\n", i);
            i++;
        }
        
        // If data point corresponds to relevant OpChannel and Slice, plot on histogram
        if (OpChannel == OpC) {
            VPos = GetVoxelPos(Voxel);
            
            if (VPos[CoordNum] == SliceNum) {
                /*if(V >=160142198 && V <160201638) {
                    continue;
                }*/
                if(V >= 160201638 && V <160260927) {
                    continue;
                }
                else {
                    if (CoordNum == 0) {
                        vishist->Fill(VPos[2], VPos[1], 0.5*Visibility);
                        reflhist->Fill(VPos[2], VPos[1], ReflVisibility);
                        bothhist->Fill(VPos[2], VPos[1], 0.5*Visibility +ReflVisibility);
                    }
                    else if (CoordNum == 1) {
                        vishist->Fill(VPos[0], VPos[2], 0.5*Visibility);
                        reflhist->Fill(VPos[0], VPos[2], ReflVisibility);
                        bothhist->Fill(VPos[0], VPos[2], 0.5*Visibility +ReflVisibility);
                    }
                    else if (CoordNum == 2) {
                        vishist->Fill(VPos[0], VPos[1], 0.5*Visibility);
                        reflhist->Fill(VPos[0], VPos[1], ReflVisibility);
                        bothhist->Fill(VPos[0], VPos[1], 0.5*Visibility +ReflVisibility);
                    }
                }
            }
        }
    }
    //cout << u << endl;
    
    // Finally, Draw the data we have plotted to canvas!
    double maxval = bothhist->GetMaximum();
    gStyle->SetOptStat(0);
    c1->cd(3);
    bothhist->SetMaximum(maxval);
    bothhist->Draw("colz");
    
    c1->cd(1);
    vishist->SetMaximum(maxval);
    vishist->Draw("col same");
    
    c1->cd(2);
    reflhist->SetMaximum(maxval);
    reflhist->Draw("col same");
    //pltree->Draw("Voxel:Visibility>>mynewhist",Form("OpChannel==%d",OpC));
}

void visibility_plane_plotter_AllChannels(int CoordNum, int SliceNum) {
    
    // Load the photon library
    TFile *fin = new TFile("dune_withfoils_lib_cleaned_ordered.root");
    
    //Load the PhotonLibraryData tree and attach variables to the neccessary branches
    TTree *pltree = (TTree*)fin->Get("PhotonLibraryData");
    int    Voxel;           pltree->SetBranchAddress("Voxel",          &Voxel);
    int    OpChannel;       pltree->SetBranchAddress("OpChannel",      &OpChannel);
    Float_t Visibility;     pltree->SetBranchAddress("Visibility",     &Visibility);
    Float_t ReflVisibility; pltree->SetBranchAddress("ReflVisibility", &ReflVisibility);
    
    // Create Canvas & Histogram Objects
    TCanvas *c1;
    TH2F *vishist;
    TH2F *reflhist;
    TH2F *bothhist;
    if (CoordNum == 0) {
        c1 = new TCanvas("c1", Form("Visibility of X Plane %i for all Optical Channels", SliceNum),900,700);
        c1->Divide(3);
        vishist  = new TH2F("vishist" , "Visible Only",     voxlimz,0,voxlimz,voxlimy,0,voxlimy);
        reflhist = new TH2F("reflhist", "Reflected Only",   voxlimz,0,voxlimz,voxlimy,0,voxlimy);
        bothhist = new TH2F("bothhist", "Both",             voxlimz,0,voxlimz,voxlimy,0,voxlimy);
    }
    else if (CoordNum == 1) {
        c1 = new TCanvas("c1", Form("Visibility of Y Plane %i for Optical Channels", SliceNum),900,700);
        c1->Divide(3);
        vishist  = new TH2F("vishist" , "Visible Only",     voxlimx,0,voxlimx,voxlimz,0,voxlimz);
        reflhist = new TH2F("reflhist", "Reflected Only",   voxlimx,0,voxlimx,voxlimz,0,voxlimz);
        bothhist = new TH2F("bothhist", "Both",             voxlimx,0,voxlimx,voxlimz,0,voxlimz);
    }
    else if (CoordNum == 2) {
        c1 = new TCanvas("c1", Form("Visibility of Z Plane %i for Optical Channels", SliceNum),900,700);
        c1->Divide(3);
        vishist  = new TH2F("vishist" , "Visible Only",     voxlimx,0,voxlimx,voxlimy,0,voxlimy);
        reflhist = new TH2F("reflhist", "Reflected Only",   voxlimx,0,voxlimx,voxlimy,0,voxlimy);
        bothhist = new TH2F("bothhist", "Both",             voxlimx,0,voxlimx,voxlimy,0,voxlimy);
    }
    
    //int u = 0;
    const int TotNum = pltree->GetEntries();
    int i = 1;
    //cout << "TotNum:\t" << TotNum << endl;
    //cout << "V\tVoxel\tOpChan\tVPosX\tVPosY\tVPosZ\n";
    
    // Iterate over all data points in tree to find relevant data
    cout << "Plotting data:\n";
    for (int V = 0; V < TotNum; V++) {
        // Get data points from tree leaves
        pltree->GetEntry(V);
        
        if ( (V % (TotNum/10) == 0) && (V != 0) ) {
            cout << Form("%i0%% Completed...\n", i);
            i++;
        }
        
        // If data point corresponds to relevant Slice, plot on histogram
        vector<int> VPos = GetVoxelPos(Voxel);
        
        if (VPos[CoordNum] == SliceNum) {
            //cout << V << "\t" << Voxel << "\t" << OpChannel << "\t" << VPos[0] << "\t" << VPos[1] << "\t" << VPos[2] << endl;
            //u++;
            if (CoordNum == 0) {
                vishist->Fill(VPos[2], VPos[1], 0.5*Visibility);
                reflhist->Fill(VPos[2], VPos[1], ReflVisibility);
                bothhist->Fill(VPos[2], VPos[1], 0.5*Visibility +ReflVisibility);
            }
            else if (CoordNum == 1) {
                vishist->Fill(VPos[0], VPos[2], 0.5*Visibility);
                reflhist->Fill(VPos[0], VPos[2], ReflVisibility);
                bothhist->Fill(VPos[0], VPos[2], 0.5*Visibility +ReflVisibility);
            }
            else if (CoordNum == 2) {
                vishist->Fill(VPos[0], VPos[1], 0.5*Visibility);
                reflhist->Fill(VPos[0], VPos[1], ReflVisibility);
                bothhist->Fill(VPos[0], VPos[1], 0.5*Visibility +ReflVisibility);
            }
        }
    }
    //cout << u << endl;
    
    // Finally, Draw the data we have plotted to canvas!
    double maxval = bothhist->GetMaximum();
    gStyle->SetOptStat(0);
    c1->cd(3);
    bothhist->SetMaximum(maxval);
    bothhist->Draw("colz");
    
    c1->cd(1);
    vishist->SetMaximum(maxval);
    vishist->Draw("col same");
    
    c1->cd(2);
    reflhist->SetMaximum(maxval);
    reflhist->Draw("col same");
    
    //pltree->Draw("Voxel:Visibility>>mynewhist",Form("OpChannel==%d",OpC));
}

void visibility_plotter_All(int CoordNum) {
    
    // Load the photon library
    TFile *fin = new TFile("dune_withfoils_lib_cleaned.root");
    
    //Load the PhotonLibraryData tree and attach variables to the neccessary branches
    TTree *pltree = (TTree*)fin->Get("PhotonLibraryData");
    int    Voxel;           pltree->SetBranchAddress("Voxel",          &Voxel);
    int    OpChannel;       pltree->SetBranchAddress("OpChannel",      &OpChannel);
    Float_t Visibility;     pltree->SetBranchAddress("Visibility",     &Visibility);
    Float_t ReflVisibility; pltree->SetBranchAddress("ReflVisibility", &ReflVisibility);
    
    // Create Canvas & Histogram Objects
    TCanvas *c1;
    TH2F *vishist;
    TH2F *reflhist;
    TH2F *bothhist;
    if (CoordNum == 0) {
        c1 = new TCanvas("c1", "Visibility along X Projection",900,700);
        c1->Divide(3);
        vishist  = new TH2F("vishist" , "Visible Only",     voxlimz,0,voxlimz,voxlimy,0,voxlimy);
        reflhist = new TH2F("reflhist", "Reflected Only",   voxlimz,0,voxlimz,voxlimy,0,voxlimy);
        bothhist = new TH2F("bothhist", "Both",             voxlimz,0,voxlimz,voxlimy,0,voxlimy);
    }
    else if (CoordNum == 1) {
        c1 = new TCanvas("c1", "Visibility along Y Projection",900,700);
        c1->Divide(3);
        vishist  = new TH2F("vishist" , "Visible Only",     voxlimx,0,voxlimx,voxlimz,0,voxlimz);
        reflhist = new TH2F("reflhist", "Reflected Only",   voxlimx,0,voxlimx,voxlimz,0,voxlimz);
        bothhist = new TH2F("bothhist", "Both",             voxlimx,0,voxlimx,voxlimz,0,voxlimz);
    }
    else if (CoordNum == 2) {
        c1 = new TCanvas("c1", "Visibility along Z Projection",900,700);
        c1->Divide(3);
        vishist  = new TH2F("vishist" , "Visible Only",     voxlimx,0,voxlimx,voxlimy,0,voxlimy);
        reflhist = new TH2F("reflhist", "Reflected Only",   voxlimx,0,voxlimx,voxlimy,0,voxlimy);
        bothhist = new TH2F("bothhist", "Both",             voxlimx,0,voxlimx,voxlimy,0,voxlimy);
    }

    const int TotNum = pltree->GetEntries();
    int i = 1;
    
    // Iterate over all data points in tree to find relevant data
    cout << "Plotting data:\n";
    for (int V = 0; V < TotNum; V++) {
        // Get data points from tree leaves
        pltree->GetEntry(V);
        
        if ( (V % (TotNum/10) == 0) && (V != 0) ) {
            cout << Form("%i0%% Completed...\n", i);
            i++;
        }
        
        // Plot on histogram
        vector<int> VPos = GetVoxelPos(Voxel);
        
        if (CoordNum == 0) {
            vishist->Fill(VPos[2], VPos[1], 0.5*Visibility);
            reflhist->Fill(VPos[2], VPos[1], ReflVisibility);
            bothhist->Fill(VPos[2], VPos[1], 0.5*Visibility +ReflVisibility);
        }
        else if (CoordNum == 1) {
            vishist->Fill(VPos[0], VPos[2], 0.5*Visibility);
            reflhist->Fill(VPos[0], VPos[2], ReflVisibility);
            bothhist->Fill(VPos[0], VPos[2], 0.5*Visibility +ReflVisibility);
        }
        else if (CoordNum == 2) {
            vishist->Fill(VPos[0], VPos[1], 0.5*Visibility);
            reflhist->Fill(VPos[0], VPos[1], ReflVisibility);
            bothhist->Fill(VPos[0], VPos[1], 0.5*Visibility +ReflVisibility);
        }
    }
    
    // Finally, Draw the data we have plotted to canvas!
    double maxval = bothhist->GetMaximum();
    gStyle->SetOptStat(0);
    c1->cd(3);
    bothhist->SetMaximum(maxval);
    bothhist->Draw("colz");
    
    c1->cd(1);
    vishist->SetMaximum(maxval);
    vishist->Draw("col same");
    
    c1->cd(2);
    reflhist->SetMaximum(maxval);
    reflhist->Draw("col same");
}

void visibility_getXZ(int PosX, int PosZ) {
    // Loop over the visibilities of a line of voxels and print them to console to check for weirdness
    TFile *fin = new TFile("dune_withfoils_lib.root");
    TTree *pltree = (TTree*)fin->Get("pmtresponse/PhotonLibraryData");
    int    Voxel;           pltree->SetBranchAddress("Voxel",          &Voxel);
    int    OpChannel;       pltree->SetBranchAddress("OpChannel",      &OpChannel);
    Float_t Visibility;     pltree->SetBranchAddress("Visibility",     &Visibility);
    Float_t ReflVisibility; pltree->SetBranchAddress("ReflVisibility", &ReflVisibility);
    
    const int TotNum = pltree->GetEntries();
    int i = 1;
    cout << "V\tVoxel\tV_y\tOpChan\tVisibility\tReflVisibility\n";
    for (int V = 0; V < TotNum; V++) {
        pltree->GetEntry(V);
        
        if ( (V % (TotNum/10) == 0) && (V != 0) ) {
            cout << Form("%i0%% Completed...\n", i);
            i++;
        }
        vector<int> VPos = GetVoxelPos(Voxel);
        if (VPos[0] == PosX && VPos[2] == PosZ) {
            cout << V << "\t" << Voxel << "\t" << VPos[1] << "\t" << OpChannel << "\t" << Visibility << "\t" << ReflVisibility << endl;
        }
    }
}

void visibility_getZRange(int MinX, int MaxX, int MinY, int MaxY, int PosZ) {
    TFile *fin = new TFile("dune_withfoils_lib_cleaned_ordered.root");
    TTree *pltree = (TTree*)fin->Get("PhotonLibraryData");
    int    Voxel;           pltree->SetBranchAddress("Voxel",          &Voxel);
    int    OpChannel;       pltree->SetBranchAddress("OpChannel",      &OpChannel);
    Float_t Visibility;     pltree->SetBranchAddress("Visibility",     &Visibility);
    Float_t ReflVisibility; pltree->SetBranchAddress("ReflVisibility", &ReflVisibility);
    
    const int TotNum = pltree->GetEntries();
    int i = 1;
    cout << "V\tVoxel\tV_y\tOpChan\tVisibility\tReflVisibility\n";
    for (int V = 0; V < TotNum; V++) {
        pltree->GetEntry(V);
        
        if ( (V % (TotNum/10) == 0) && (V != 0) ) {
            cout << Form("%i0%% Completed...\n", i);
            i++;
        }
        vector<int> VPos = GetVoxelPos(Voxel);
        if (VPos[0] >= MinX && VPos[0] < MaxX && VPos[1] >= MinY && VPos[1] < MaxY && VPos[2] == PosZ) {
            cout << V << "\t" << Voxel << "\t" << VPos[1] << "\t" << OpChannel << "\t" << Visibility << "\t" << ReflVisibility << endl;
        }
    }
}

void visibility_anomaly_compare(int OpC) {
    TFile *fin = new TFile("dune_withfoils_lib.root");
    TTree *pltree = (TTree*)fin->Get("pmtresponse/PhotonLibraryData");
    int    Voxel;           pltree->SetBranchAddress("Voxel",          &Voxel);
    int    OpChannel;       pltree->SetBranchAddress("OpChannel",      &OpChannel);
    Float_t Visibility;     pltree->SetBranchAddress("Visibility",     &Visibility);
    Float_t ReflVisibility; pltree->SetBranchAddress("ReflVisibility", &ReflVisibility);
    
    TCanvas *c1 = new TCanvas("c1", "Visibility difference for Z plane 155",900,700);
    c1->Divide(1,2);
    TH1F *vishist  = new TH1F("vishist" , "Visible Only",  300, -0.1, 0.1);  
    TH1F *reflhist = new TH1F("reflhist", "Reflected Only",300, -0.002, 0.002);
    
    vector<int> VPos;
    for (/*int V = 160142198; V<160201638;*/int V = 161100801; V<161163574; V++) {
        pltree->GetEntry(V);
        if(OpChannel == OpC) {
            VPos = GetVoxelPos(Voxel);
            vishist ->Fill(Visibility);
            reflhist->Fill(ReflVisibility);
        }
    }
    for (/*int V = 160201638; V<160260927;*/int V = 161279726; V<161342128; V++) {
        pltree->GetEntry(V);
        if(OpChannel == OpC) {
            VPos = GetVoxelPos(Voxel);
            vishist ->Fill(-1*Visibility);
            reflhist->Fill(-1*ReflVisibility);
        }
    }
    
    //gStyle->SetOptStat(0);
    c1->cd(1);
    vishist->Draw("colz");

    c1->cd(2);
    reflhist->Draw("colz");
}

void visibility_compare_to_Poisson(int OpC, int y, int xmin, int xmax, int zmin, int zmax) {
    // 
    TFile *fin = new TFile("dune_withfoils_lib_cleaned_ordered.root");
    TTree *pltree = (TTree*)fin->Get("PhotonLibraryData");
    int    Voxel;           pltree->SetBranchAddress("Voxel",          &Voxel);
    int    OpChannel;       pltree->SetBranchAddress("OpChannel",      &OpChannel);
    Float_t Visibility;     pltree->SetBranchAddress("Visibility",     &Visibility);
    //Float_t ReflVisibility; pltree->SetBranchAddress("ReflVisibility", &ReflVisibility);
    
    const int TotNum = pltree->GetEntries();
    const int NumPhot = 500000;
    const int NumVoxels = (xmax-xmin)*(zmax-zmin);
    int i = 0;
    int j = 1;
    Float_t sumv = 0;
    for (int V = 0; V < TotNum; V++) {
        pltree->GetEntry(V);
        
        if ( (V % (TotNum/10) == 0) && (V != 0) ) {
            cout << Form("%i0%% Completed...\n", j);
            j++;
        }
        
        if (OpChannel == OpC) {
            vector<int> VPos = GetVoxelPos(Voxel);
            if(VPos[1] == y && VPos[0] >= xmin && VPos[0] < xmax && VPos[2] >= zmin && VPos[2] < zmax && Visibility > 0.) {
                i++;
                sumv += Visibility;
                
            }
        }
    }
    
    int n0 = NumVoxels - i;
    Float_t mean_vis = sumv / NumVoxels;
    Float_t lambda = NumPhot * mean_vis;
    Float_t n0_exp = NumVoxels * exp(-lambda);
    
    cout << "Results:\n";
    cout << Form("Number of voxels sampled:\t%i\nNumber of non-empty entries:\t%i\nNumber of empty entries:\t%i\n",NumVoxels,i,n0);
    cout << Form("Mean visibility:\t%f\nlambda:\t%f\n",mean_vis, lambda);
    cout << Form("\t=> Number of expected zero entries assuming Poissonian statistics:%f\n",n0_exp);
    
}

void num_plane_plotter_AllChannels(int CoordNum, int SliceNum) {
    // Does similar to visibility_plane_plotter_Allchannels(), but plots simply the number
    // of entries and nothing else.
    
    // Load the photon library
    TFile *fin = new TFile("dune_withfoils_lib.root");
    
    //Load the PhotonLibraryData tree and attach variables to the neccessary branches
    TTree *pltree = (TTree*)fin->Get("pmtresponse/PhotonLibraryData");
    int    Voxel;           pltree->SetBranchAddress("Voxel",          &Voxel);
    
    // Create Canvas & Histogram Objects
    TCanvas *c1;
    TH2F *vishist;
    TH2F *reflhist;
    if (CoordNum == 0) {
        c1 = new TCanvas("c1", Form("Number of Entries in X Plane %i", SliceNum),900,700);
        c1->Divide(2);
        vishist  = new TH2F("vishist" , "Visible Only",     voxlimz,0,voxlimz,voxlimy,0,voxlimy);
        reflhist = new TH2F("reflhist", "Reflected Only",   voxlimz,0,voxlimz,voxlimy,0,voxlimy);
    }
    else if (CoordNum == 1) {
        c1 = new TCanvas("c1", Form("Number of Entries in Y Plane %i", SliceNum),900,700);
        c1->Divide(2);
        vishist  = new TH2F("vishist" , "Visible Only",     voxlimx,0,voxlimx,voxlimz,0,voxlimz);
        reflhist = new TH2F("reflhist", "Reflected Only",   voxlimx,0,voxlimx,voxlimz,0,voxlimz);
    }
    else if (CoordNum == 2) {
        c1 = new TCanvas("c1", Form("Number of Entries in Z Plane %i", SliceNum),900,700);
        c1->Divide(2);
        vishist  = new TH2F("vishist" , "Visible Only",     voxlimx,0,voxlimx,voxlimy,0,voxlimy);
        reflhist = new TH2F("reflhist", "Reflected Only",   voxlimx,0,voxlimx,voxlimy,0,voxlimy);
    }
    //int u = 0;
    const int TotNum = pltree->GetEntries();
    int i = 1;
    //cout << "TotNum:\t" << TotNum << endl;
    //cout << "V\tVoxel\tOpChan\tVPosX\tVPosY\tVPosZ\n";
    
    // Iterate over all data points in tree to find relevant data
    cout << "Plotting data:\n";
    for (int V = 0; V < TotNum; V++) {
        // Get data points from tree leaves
        pltree->GetEntry(V);
        
        if ( (V % (TotNum/10) == 0) && (V != 0) ) {
            cout << Form("%i0%% Completed...\n", i);
            i++;
        }
        
        // If data point corresponds to relevant Slice, plot on histogram
        vector<int> VPos = GetVoxelPos(Voxel);
        
        if (VPos[CoordNum] == SliceNum) {
            if (CoordNum == 0) {
                vishist->Fill(VPos[2], VPos[1]);
                reflhist->Fill(VPos[2], VPos[1]);
            }
            else if (CoordNum == 1) {
                vishist->Fill(VPos[0], VPos[2]);
                reflhist->Fill(VPos[0], VPos[2]);
            }
            else if (CoordNum == 2) {
                vishist->Fill(VPos[0], VPos[1]);
                reflhist->Fill(VPos[0], VPos[1]);
            }
        }
        
    }

    // Finally, Draw the data we have plotted to canvas!
    gStyle->SetOptStat(0);
    c1->cd(2);
    reflhist->Draw("colz");

    c1->cd(1);
    vishist->Draw("col same");

}

void LightYieldDriftDistancePlotter(int y =62, int z = 150) {
    // Produce a 1D histogram plot showing the drift-distance dependence of the
    // light yield of the detector.
    const double xlength = 379.662; //cm
    const double voxlen = xlength/voxlimx; //cm
    const double N = 500000.;
    // Load the photon library
    TFile *fin = new TFile("dune_withfoils_lib.root");
    
    //Load the PhotonLibraryData tree and attach variables to the neccessary branches
    TTree *pltree = (TTree*)fin->Get("pmtresponse/PhotonLibraryData");
    int    Voxel;           pltree->SetBranchAddress("Voxel",          &Voxel);
    int    OpChannel;       pltree->SetBranchAddress("OpChannel",      &OpChannel);
    Float_t Visibility;     pltree->SetBranchAddress("Visibility",     &Visibility);
    Float_t ReflVisibility; pltree->SetBranchAddress("ReflVisibility", &ReflVisibility);
    
    // Create Canvas & Histogram Objects
    TCanvas *c1 = new TCanvas("c1", "Light Yield at Detector's Centre",900,700);
    TH1F *vishist = new TH1F("vishist" , "Visible Only",     voxlimx,0,xlength);
    TH1F *reflhist = new TH1F("reflhist", "Reflected Only",   voxlimx,0,xlength);
    TH1F *bothhist = new TH1F("bothhist", "Both",             voxlimx,0,xlength);
    
    const int TotNum = pltree->GetEntries();
    int i = 1;

    // Iterate over all data points in tree to find relevant data
    cout << "Plotting data:\n";
    for (int V = 0; V < TotNum; V++) {
        // Get data points from tree leaves
        pltree->GetEntry(V);
        
        if ( (V % (TotNum/10) == 0) && (V != 0) ) {
            cout << Form("%i0%% Completed...\n", i);
            i++;
        }
        
        // If data point corresponds to relevant Slice, plot on histogram
        vector<int> VPos = GetVoxelPos(Voxel);
        
        if (VPos[1] == y && VPos[2] == z) {
            vishist->Fill(VPos[0]*voxlen +0.5*voxlen, 0.5*Visibility);
            reflhist->Fill(VPos[0]*voxlen +0.5*voxlen, ReflVisibility);
            bothhist->Fill(VPos[0]*voxlen +0.5*voxlen, 0.5*Visibility +ReflVisibility);
        }
    }

    // Finally, Draw the data we have plotted to canvas! Also do some fancying of plot design
    gStyle->SetOptStat(0);
    gStyle->SetEndErrorSize(3);
    gStyle->SetErrorX(0);
    
    for(int j=0; j<voxlimx; j++) { //This sets the error bars to the right values
        vishist->SetBinError (j, sqrt(vishist ->GetBinContent(j)/N) );
        reflhist->SetBinError(j, sqrt(reflhist->GetBinContent(j)/N) );
        bothhist->SetBinError(j, sqrt(bothhist->GetBinContent(j)/N) );
    }
    
    vishist ->Scale(24000);
    reflhist->Scale(24000);
    bothhist->Scale(24000);
    
    vishist ->SetAxisRange(10,350);
    reflhist->SetAxisRange(10,350);
    bothhist->SetAxisRange(10,350);
    bothhist->SetMaximum(1500);
    
    vishist ->SetLineColor(kBlack);
    reflhist->SetLineColor(kRed);
    bothhist->SetLineColor(kBlue);
    
    vishist->Draw("E1");
    reflhist->Draw("E1 same");
    bothhist->Draw("E1 same");
    
    gPad->BuildLegend();
}

void LYDistanceSumPlanes() {
    // Produce a 1D histogram plot showing the drift-distance dependence of the
    // light yield of the detector, summing over a given x-plane for each value.
    const double xlength = 379.662; //cm
    const double voxlen = xlength/voxlimx; //cm
    const double N = 500000.;
    // Load the photon library
    TFile *fin = new TFile("dune_withfoils_lib.root");
    
    //Load the PhotonLibraryData tree and attach variables to the neccessary branches
    TTree *pltree = (TTree*)fin->Get("pmtresponse/PhotonLibraryData");
    int    Voxel;           pltree->SetBranchAddress("Voxel",          &Voxel);
    /*int    OpChannel;       pltree->SetBranchAddress("OpChannel",      &OpChannel);*/
    Float_t Visibility;     pltree->SetBranchAddress("Visibility",     &Visibility);
    Float_t ReflVisibility; pltree->SetBranchAddress("ReflVisibility", &ReflVisibility);
    
    // Create Canvas
    TCanvas *c1 = new TCanvas("c1", "Light Yield at Detector's Centre",900,700);
    
    // Create histogram arrays ***THIS MIGHT BE A TERRIBLE IDEA!!***
    vector<TH1F*> vishista;
    vector<TH1F*> reflhista;
    vector<TH1F*> bothhista;
    for (int j=0; j<voxlimx; j++) {
        TH1F *vh = new TH1F(Form("vh%i",j), Form("Visible Only %i",j),     voxlimx,0,xlength);
        TH1F *rh = new TH1F(Form("rh%i",j), Form("Reflected Only %i",j),   voxlimx,0,xlength);
        TH1F *bh = new TH1F(Form("bh%i",j), Form("Both %i",j),             voxlimx,0,xlength);
        vishista.push_back(vh);
        reflhista.push_back(rh);
        bothhista.push_back(bh);
    }

    
    const int TotNum = pltree->GetEntries();
    int i = 1;
    
    // Iterate over all data points in tree to find relevant data
    cout << "Plotting data:\n";
    for (int V = 0; V < TotNum; V++) {
        // Get data points from tree leaves
        pltree->GetEntry(V);
        //Print a 10% Completed... etc.
        if ( (V % (TotNum/10) == 0) && (V != 0) ) {
            cout << Form("%i0%% Completed...\n", i);
            i++;
        }
        
        // Get position
        vector<int> VPos = GetVoxelPos(Voxel);
        int x = VPos[0];
        
        // Fill data to relevant histograms
        vishista[x] ->Fill(x*voxlen +0.5*voxlen, 0.5*Visibility);
        reflhista[x]->Fill(x*voxlen +0.5*voxlen, ReflVisibility);
        bothhista[x]->Fill(x*voxlen +0.5*voxlen, 0.5*Visibility +ReflVisibility);
    }
    /*
    // Finally, Draw the data we have plotted to canvas! Also do some fancying of plot design
    gStyle->SetOptStat(0);
    gStyle->SetEndErrorSize(3);
    gStyle->SetErrorX(0);
    
    for(int j=0; j<voxlimx; j++) { //This sets the error bars to the right values
        vishist->SetBinError (j, sqrt(vishist ->GetBinContent(j)/N) );
        reflhist->SetBinError(j, sqrt(reflhist->GetBinContent(j)/N) );
        bothhist->SetBinError(j, sqrt(bothhist->GetBinContent(j)/N) );
    }
    double scale = 24000./(voxlimy*voxlimz); // 2.4e4 factor for vis->LY, disivion for mean
    vishist ->Scale(scale);
    reflhist->Scale(scale);
    bothhist->Scale(scale);
    
    vishist ->SetAxisRange(10,350);
    reflhist->SetAxisRange(10,350);
    bothhist->SetAxisRange(10,350);
    bothhist->SetMaximum(1500);*/
    
    vishista[20] ->SetLineColor(kBlack);
    reflhista[20]->SetLineColor(kRed);
    bothhista[20]->SetLineColor(kBlue);
    
    vishista[20] ->Draw();
    reflhista[20]->Draw("same");
    bothhista[20]->Draw("same");
    
    /*vishist->Draw("E1");
    reflhist->Draw("E1 same");
    bothhist->Draw("E1 same");*/
    
    gPad->BuildLegend();
    
    /*
    // Create a 2D array to hold data about relevant statistics
    // Format of array: row due to each x plane, 8 columns for: numcounted;sum_v;sum_(v^2).
    // 3 columns for each of last two due to three visibility sets to worry about.
    vector<vector<double>> vis_stats;
    vis_stats.resize(voxlimx, vector<double>(7,0));
    
    vishist->Fill(x*voxlen +0.5*voxlen, 0.5*Visibility);
        reflhist->Fill(x*voxlen +0.5*voxlen, ReflVisibility);
        bothhist->Fill(x*voxlen +0.5*voxlen, 0.5*Visibility +ReflVisibility);
        
        // Add moments to array
        vis_stats[x][0]++;
        vis_stats[x][1] += 0.5*Visibility;
        vis_stats[x][2] += 0.25*Visibility*Visibility;
        vis_stats[x][3] += ReflVisibility;
        vis_stats[x][4] += ReflVisibility*ReflVisibility;
        vis_stats[x][5] += 0.5*Visibility + ReflVisibility;
        vis_stats[x][6] += pow(0.5*Visibility + ReflVisibility,2); */
}
