//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: B1DetectorConstruction.cc 75117 2013-10-28 09:38:37Z gcosmo $
//
/// \file B1DetectorConstruction.cc
/// \brief Implementation of the B1DetectorConstruction class

#include "DetectorConstruction.hh"

#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Polyhedra.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4AssemblyTriplet.hh"
#include "G4MultiUnion.hh"
#include "G4VisAttributes.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction()
: G4VUserDetectorConstruction(),
  fScoringVolume(0)
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{ }

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  // Get nist material manager
  G4NistManager* nist = G4NistManager::Instance();

  // Option to switch on/off checking of volumes overlaps
  //
  G4bool checkOverlaps = true;

  //
  // World+
  //
  G4double world_sizeXYZ = 0.5*m;
  G4Material* world_mat = nist->FindOrBuildMaterial("G4_AIR");

  G4Box* solidWorld =
    new G4Box("World",                       //its name
       1*world_sizeXYZ, 1*world_sizeXYZ, 1*world_sizeXYZ);     //its size

  G4LogicalVolume* logicWorld =
    new G4LogicalVolume(solidWorld,          //its solid
                        world_mat,           //its material
                        "World");            //its name

  G4VPhysicalVolume* physWorld =
    new G4PVPlacement(0,                     //no rotation
                      G4ThreeVector(),       //at (0,0,0)
                      logicWorld,            //its logical volume
                      "World",               //its name
                      0,                     //its mother  volume
                      false,                 //no boolean operation
                      0,                     //copy number
                      checkOverlaps);        //overlaps checking

// Germanium.
G4Material* det_material = nist->FindOrBuildMaterial("G4_Ge");

//Hexagonal prisms
//Variables
G4double Pi=3.14159265359;
G4double phiStart = Pi/6;
G4double phiTotal = 2*Pi;
G4int numSide = 6; //number of sides
G4int numZPlanes = 6; //number of planes
const G4double r_hex = 30.74*mm; //Distance from center to inner surface, r_hex=(sqrt(3)/2)*R_hex
const G4double R_hex = 35.5*mm; //Half of Hexagon's Diagonal
const G4double z_height = 10*cm; //Height of Hexagon
const G4double rInner[] ={0, 0, 0, 0, 0, 0}; //Tangent Distance to inner surface. No thickness
const G4double rOuter[] = {r_hex, r_hex, r_hex, r_hex, r_hex, r_hex}; //Tangent Distance to outer surface.
const G4double zPlane[] = {0., 0., 0., z_height, z_height, z_height}; //Position of Z-planes
//Rotation Matrix -> No Rotation
G4RotationMatrix rotm  = G4RotationMatrix();
G4ThreeVector position1 = G4ThreeVector(-r_hex,-0.5*R_hex,-z_height/2); //1st Hexagon Position
G4ThreeVector position2 = G4ThreeVector(r_hex,-0.5*R_hex,-z_height/2); //2nd Hexagon Position
G4ThreeVector position3 = G4ThreeVector(0.,R_hex,-z_height/2); //3rd Hexagon Position (Upper one)

//Solid (all three hexagons are the same)
G4Polyhedra* solidPoly1 = new G4Polyhedra("Polyhedron1",phiStart, phiTotal,numSide,numZPlanes,zPlane,rInner,rOuter);
G4Polyhedra* solidPoly2 = new G4Polyhedra("Polyhedron2",phiStart, phiTotal,numSide,numZPlanes,zPlane,rInner,rOuter);
G4Polyhedra* solidPoly3 = new G4Polyhedra("Polyhedron3",phiStart, phiTotal,numSide,numZPlanes,zPlane,rInner,rOuter);
//Displacements for the shapes
G4Transform3D tr1 = G4Transform3D(rotm,position1);
G4Transform3D tr2 = G4Transform3D(rotm,position2);
G4Transform3D tr3 = G4Transform3D(rotm,position3);
//Multi-Union (structure) of 3 Hexagons
G4MultiUnion* hex_detector = new G4MultiUnion("Hexagons_Union");
hex_detector->AddNode(*solidPoly1,tr1);
hex_detector->AddNode(*solidPoly2,tr2);
hex_detector->AddNode(*solidPoly3,tr3);
//Close the Structure
hex_detector->Voxelize();
//Logical Volume for the structure, as if it was a normal solid.
G4LogicalVolume* hex_detectorLV = new G4LogicalVolume(hex_detector,det_material,"Hexagons_UnionLV");

//Placement of the Volume in the World.
G4double dist=11.2*cm; //Distance from start to the "face" of the detector
G4double dist_x=(sqrt(2)/2)*dist;
G4double dist_z=(sqrt(2)/2)*dist;

G4RotationMatrix* rot = new G4RotationMatrix();
rot->rotateY(45.*deg); //Rotation of the whole detector.
new G4PVPlacement(rot,G4ThreeVector(dist_x,0.,-dist_z),hex_detectorLV,"Hexagons_UnionLV",logicWorld,false,0,checkOverlaps);

G4RotationMatrix* rot2 = new G4RotationMatrix();
rot2->rotateY(315.*deg); //Rotation of the whole detector.
new G4PVPlacement(rot2,G4ThreeVector(-dist_x,0.,-dist_z),hex_detectorLV,"Hexagons_UnionLV",logicWorld,false,0,checkOverlaps);

G4RotationMatrix* rot3 = new G4RotationMatrix();
rot3->rotateY(135.*deg);
new G4PVPlacement(rot3,G4ThreeVector(dist_x,0.,dist_z),hex_detectorLV,"Hexagons_UnionLV",logicWorld,false,0,checkOverlaps);

G4RotationMatrix* rot4 = new G4RotationMatrix();
rot4->rotateY(225.*deg);
new G4PVPlacement(rot4,G4ThreeVector(-dist_x,0.,dist_z),hex_detectorLV,"Hexagons_UnionLV",logicWorld,false,0,checkOverlaps);



//Visuals
G4VisAttributes* aVisAtt= new G4VisAttributes(G4Colour(0,1.0,1.0));
hex_detectorLV->SetVisAttributes(aVisAtt);
G4VisAttributes* bVisAtt= new G4VisAttributes(G4Colour(1.0,1.0,1.0));
logicWorld->SetVisAttributes(bVisAtt);

//The same Structure but with separate placements. Can't Treat the detector as a whole though.

//G4Polyhedra* solidPoly1 = new G4Polyhedra("Polyhedron1",phiStart, phiTotal,numSide,numZPlanes,zPlane,rInner,rOuter);
//G4LogicalVolume* logicPoly = new G4LogicalVolume(solidPoly1,det_material,"Polyhedron");
//Placement
//new G4PVPlacement(0,G4ThreeVector(0,0,-20*cm),logicPoly,"Polyhedron",logicWorld,false,3,checkOverlaps);//Simple placement. Just one
//new G4PVPlacement(0,G4ThreeVector(-r_hex,-0.5*R_hex,-20*cm),logicPoly,"Polyhedron",logicWorld,false,3,checkOverlaps);//1st hexagon placement
//new G4PVPlacement(0,G4ThreeVector(r_hex,-0.5*R_hex,-20*cm),logicPoly,"Polyhedron",logicWorld,false,0,checkOverlaps);//2nd hexagon placement
//new G4PVPlacement(0,G4ThreeVector(0.,R_hex,-20*cm),logicPoly,"Polyhedron",logicWorld,false,0,checkOverlaps);//3rd hexagon placement

  //
  fScoringVolume = hex_detectorLV;

  //
  //always return the physical World
  //
  return physWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
