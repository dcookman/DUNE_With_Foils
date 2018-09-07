// LY_plotter_from_MC.cc
// Takes the histograms formed by the SN MC simulation and creates a drift-distance plot
// Daniel Cookman 2018/07

void LY_drift_dist_plotter(const char * infilename) {
    const double voxlimx = 75.;
    const double xlength = 379.662; //cm
    const double voxlen = xlength/voxlimx; //cm
    
    TFile *fin = new TFile(infilename);
    
    // Create Canvas & Histogram Objects
    TCanvas *c1 = new TCanvas("c1", "Light Yield Plot",900,700);
    TH1F *vishist  = new TH1F("vishist" , "VUV",    voxlimx,0,xlength);
    TH1F *reflhist = new TH1F("reflhist" , "Visible",voxlimx,0,xlength);
    TH1F *bothhist = new TH1F("bothhist", "Both",   voxlimx,0,xlength);
    cout << "Doing hist stuff...\n";
    for (int vx=0; vx<75; vx++) {
        //cout << "Testing...\n";
        TH1F *vh; fin->GetObject(Form("vh%i;1",vx),vh);
        TH1F *rh; fin->GetObject(Form("rh%i;1",vx),rh);
        TH1F *bh; fin->GetObject(Form("bh%i;1",vx),bh);
        
        vishist-> Fill(vx*voxlen +0.5*voxlen, vh->GetMean());
        reflhist->Fill(vx*voxlen +0.5*voxlen, rh->GetMean());
        bothhist->Fill(vx*voxlen +0.5*voxlen, bh->GetMean());
        //cout << "After Fill...\n";
        
        vishist-> SetBinError(vx+1, vh->GetStdDev());
        reflhist->SetBinError(vx+1, rh->GetStdDev());
        bothhist->SetBinError(vx+1, bh->GetStdDev());
        
        //cout << Form("ENTRY:\t%i\tX Val:\t%g\tLYs (vis, both):\t(%g+/-%g);\t(%g+/-%g)\n",vx,vx*voxlen +0.5*voxlen,vh->GetMean(),vh->GetStdDev(),bh->GetMean(),bh->GetStdDev());
    }
    
    gStyle->SetOptStat(0);
    gStyle->SetOptTitle(0);
    gStyle->SetEndErrorSize(3);
    gStyle->SetErrorX(0);

    vishist ->SetAxisRange(10,325);
    reflhist ->SetAxisRange(10,325);
    bothhist->SetAxisRange(10,325);
    
    vishist ->SetLineColor(kBlue);
    reflhist->SetLineColor(kRed);
    bothhist->SetLineColor(kBlack);
    
    vishist ->SetMarkerColor(kBlue);
    reflhist->SetMarkerColor(kRed);
    bothhist->SetMarkerColor(kBlack);
    
    vishist ->SetMarkerStyle(kFullCircle);
    reflhist->SetMarkerStyle(kFullStar);
    bothhist->SetMarkerStyle(kFullCross);
    
    vishist->GetXaxis()->SetTitle("Drift Distance (cm)");
    vishist->GetYaxis()->SetTitle("Light Yield (photons/MeV)");
    //vishist ->Scale(scale);
    //reflhist->Scale(scale);
    //bothhist->Scale(scale);
    
    vishist->Draw("E1");
    reflhist->Draw("E1 same");
    bothhist->Draw("E1 same");
    
    gPad->BuildLegend();
}
