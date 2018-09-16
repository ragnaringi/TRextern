//
//  counter.cpp
//  counter
//
//  Created by Ragnar Hrafnkelsson on 01/12/2017.
//  Copyright © 2017 Reactify. All rights reserved.
//

// This is based on the [counter] example from IOhannes m zmölnig's Pd External HOWTO
// http://pdstatic.iem.at/externals-HOWTO/pd-externals-HOWTOse4.html

#include "TRextern.h"

class counter : public TRextern {
public:
  void  setup( int argc, t_atom *argv ) override;
  void  exit() override;
  
  void  bangReceived  ( InletRef inlet ) override;
  void  floatReceived ( InletRef inlet, t_sample value ) override;
  void  symbolReceived( InletRef inlet, t_symbol* symbol ) override;

  int   mCount;
  float mStep;
  int   mDown, mUp;
};

//------------------------------------------------------------------------------
void counter::setup( int argc, t_atom *argv ) {
  float f1 = 0, f2 = 0;
  
  // If three arguments are passed, these should be the lower boundary, the upper boundary and the step width.

  mStep = 1;
  switch( argc ){
    default:
    case 3:
      mStep = atom_getfloat(argv+2);
    case 2:
      f2 = atom_getfloat(argv+1);
    case 1:
      f1 = atom_getfloat(argv);
      break;
    case 0:
      break;
  }
  
  if ( argc < 2 ) f2 = f1;
  
  mDown = (f1 < f2) ? f1 : f2;
  mUp   = (f1 > f2) ? f1 : f2;

  mCount = mDown;
  
  // Set up all inlets and outlets here
  addInletBang("Bang");
  // TODO: addInletList("Bounds");
  addInletFloat("Step");
  
  addOutlet("Float");
  addOutlet("Bang");
  
  // TODO: Add "set" and "reset" messages to first inlet
}

//------------------------------------------------------------------------------
void counter::exit() {
  // Free any resources
}

//------------------------------------------------------------------------------
void counter::bangReceived( InletRef inlet ) {
  float f = mCount;
  
  mCount += mStep;
  if ( mDown - mUp ) {
    if ( (mStep > 0) && (mCount > mUp) ) {
      mCount = mDown;
      getOutlets()[1]->sendBang();
    }
    else if ( mCount < mDown ) {
      mCount = mUp;
      getOutlets()[1]->sendBang();
    }
  }
  
  getOutlets()[0]->sendFloat(f);
}

//------------------------------------------------------------------------------
void counter::floatReceived( InletRef inlet, t_sample value ) {
  mStep = value;
}

//------------------------------------------------------------------------------
void counter::symbolReceived( InletRef inlet, t_symbol* symbol ) {
  post("Symbol received: %s = %s", inlet->getId().c_str(), symbol->s_name );
}

TREXTERN_CREATE(counter)
