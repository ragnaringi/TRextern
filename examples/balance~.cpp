//
//  balance_tilde.cpp
//  balance~
//
//  Created by Ragnar Hrafnkelsson on 01/12/2017.
//  Copyright © 2017 Reactify. All rights reserved.
//

// This is based on the [pan~] example from IOhannes m zmölnig's Pd External HOWTO
// http://pdstatic.iem.at/externals-HOWTO/pd-externals-HOWTOse5.html#x7-290005

#include "TRextern.h"

class balance_tilde : public TRextern {
public:
  void  setup( int argc, t_atom *argv ) override;
  void  exit() override;
  
  void  bangReceived  ( InletRef inlet ) override;
  void  intReceived   ( InletRef inlet, long value ) override;
  void  floatReceived ( InletRef inlet, t_sample value ) override;
  void  symbolReceived( InletRef inlet, t_symbol* symbol ) override;
  
  void  process( t_sample **const inChannels, t_sample **const outChannels, long size ) override;

  t_sample mBalance;
};

//------------------------------------------------------------------------------
void balance_tilde::setup( int argc, t_atom *argv ) {
  if ( argc == 1 ) {
    mBalance = atom_getfloat( argv );
  } else {
    mBalance = 0.f;
  }

  // Set up all inlets and outlets here
  setupIO(2, 1);
  addInletFloat("balance");
}

//------------------------------------------------------------------------------
void balance_tilde::exit() {
  // Free any resources
}

//------------------------------------------------------------------------------
void balance_tilde::bangReceived( InletRef inlet ) {
  post("Bang received: %s", inlet->getId().c_str() );
}

//------------------------------------------------------------------------------
void balance_tilde::intReceived( InletRef inlet, long value ) {
  post("Int received: %s = %i", inlet->getId().c_str(), value );
}

//------------------------------------------------------------------------------
void balance_tilde::floatReceived( InletRef inlet, t_sample value ) {
  post("Float received: %s = %f", inlet->getId().c_str(), value );
  mBalance = value;
}

//------------------------------------------------------------------------------
void balance_tilde::symbolReceived( InletRef inlet, t_symbol* symbol ) {
  post("Symbol received: %s = %s", inlet->getId().c_str(), symbol->s_name );
}

//------------------------------------------------------------------------------
void balance_tilde::process( t_sample **const inBuffers, t_sample **const outBuffers, long size ) {
  t_sample *in1 = inBuffers[0];
  t_sample *in2 = inBuffers[1];
  t_sample *out = outBuffers[0];
  long      n   = size;
  t_sample  balance = (mBalance<0)?0.0:(mBalance>1)?1.0:mBalance;

  while (n--) *out++ = (*in1++)*(1-balance)+(*in2++)*balance;
}

TREXTERN_CREATE(balance_tilde)
