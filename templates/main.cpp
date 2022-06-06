//
//  __template__.cpp
//  __template__
//
//  Created by [Author].
//  Copyright © 2018 [Company]. All rights reserved.
//

#include "TRextern.h"

class __template__ : public TRextern {
public:
  void  setup( int argc, t_atom *argv ) override;
  void  exit() override;
  
  void  bangReceived  ( InletRef inlet ) override;
  void  floatReceived ( InletRef inlet, t_sample value ) override;
  void  symbolReceived( InletRef inlet, t_symbol* symbol ) override;
};

//------------------------------------------------------------------------------
void __template__::setup( int argc, t_atom *argv ) {
  switch( argc ){
    default:
    case 3:
      post("Arg 3: %f", atom_getfloat(argv+2));
    case 2:
      post("Arg 2: %f", atom_getfloat(argv+1));
    case 1:
      post("Arg 1: %f", atom_getfloat(argv));
      break;
    case 0:
      break;
  }
  
  // Set up all inlets and outlets here
  addInletBang("Bang Inlet");
  addInletFloat("Float Inlet");
}

//------------------------------------------------------------------------------
void __template__::exit() {
  // Free any resources
}

//------------------------------------------------------------------------------
void __template__::bangReceived( InletRef inlet ) {
  post("Bang received from: %s", inlet->getId().c_str());
}

//------------------------------------------------------------------------------
void __template__::floatReceived( InletRef inlet, t_sample value ) {
  post("Float %f received from: %s", value, inlet->getId().c_str());
}

//------------------------------------------------------------------------------
void __template__::symbolReceived( InletRef inlet, t_symbol* symbol ) {
  post("Symbol %s received from %s", symbol->s_name, inlet->getId().c_str());
}

TREXTERN_CREATE(__template__)
