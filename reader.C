{
gROOT->Reset();
#include "Riostream.h"

  ifstream in;
  in.open("out.dat");
  Double_t x, sum, evt;
  sum =0;
  Int_t nlines=0;
  TH1F *h1 = new TH1F("h1","x distribution",8196,0.,2.);

  TRandom3 r;
  while(!in.eof()){
    in >> evt>> x;
    x=r.Gaus(x,0.0012);
    h1->Fill(x);
    nlines++;

  }
  in.close();

  h1->SetTitle(";Energy (MeV);Counts");

  TPaveStats *canv = new TPaveStats();
  gStyle->SetOptStat(1111111);
  //Background Removal
  a = h1->ShowBackground(20,"nocompton");
   h1->Add(a,-1);
  h1->Draw();








}
