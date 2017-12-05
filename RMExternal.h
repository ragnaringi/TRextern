//
//  RMExternal.h
//  RMExternal
//
//  Created by Ragnar Hrafnkelsson on 01/12/2017.
//  Copyright © 2017 Reactify. All rights reserved.
//

#pragma once

#include <assert.h>
#include <vector>
#include <string>
#include "m_pd.h"

//! Pointer to class
static t_class *m_class;

struct RMExternal;

//! Class dataspace
typedef struct _external {
  t_object      x_obj; // Internal object-properties
  RMExternal*   impl;
} t_external;


//! Base external object
struct RMExternal {
public:
  t_object* mObject; // Pointer to internal object-properties. Do not use.
  
  RMExternal();
  virtual ~RMExternal();
  
  //! Override to perfom setup
  virtual void  setup( int argc, t_atom *argv ) {}
  //! Override to free resources before exit
  virtual void  exit() {}
  //! Override to process audio
  virtual void  process( float ** inChannels, float ** outChannels, int size ) {};

  // Setup audio inlets and outlets
  virtual void  setupIO( int inChannels, int outChannels ) final;
  
  // Get number of audio ins/outs
  int const inChannelCount()  const { return mInChannels; }
  int const outChannelCount() const { return mOutChannels; }
  
  // Control in/out
  void      addInletBang(void);
  void      addInletFloat( float *f = nullptr );
  void      addInletSymbol();
  
protected:
  
private:
  void    cleanup();
  int     mInChannels;
  int     mOutChannels;
  std::vector<t_inlet *>  mInlets;
  std::vector<t_outlet *> mOutlets;
};


// Forward declarations
void  ext_dsp( t_external *x, t_signal **sp );
void *ext_new( t_symbol *s, int argc, t_atom *argv );


//------------------------------------------------------------------------------
void ext_float( t_external *x, float f ) {
  post("float = %f", f);
}

void ext_bang( t_external *x ) {
  post("bang");
}

//------------------------------------------------------------------------------
RMExternal::RMExternal() : mInChannels(0), mOutChannels(0) {}

//------------------------------------------------------------------------------
RMExternal::~RMExternal() {
  cleanup();
}

//------------------------------------------------------------------------------
void RMExternal::setupIO( int inChannels, int outChannels ) {
  class_addmethod( m_class, (t_method)ext_dsp, gensym("dsp"), A_NULL );
  
  for ( auto i = 0; i < inChannels; i++ ) {
    auto inlet = inlet_new( mObject, &mObject->ob_pd, &s_signal, &s_signal );
    mInlets.push_back( inlet );
  }
  
  for ( auto i = 0; i < outChannels; i++ ) {
    mOutlets.push_back( outlet_new( mObject, &s_signal ) );
  }
  
  mInChannels  = inChannels;
  mOutChannels = outChannels;
}

//------------------------------------------------------------------------------
void RMExternal::addInletBang() {
  inlet_new( mObject, &mObject->ob_pd, &s_bang, &s_bang );
  class_addbang( m_class, ext_bang );
}

//------------------------------------------------------------------------------
void RMExternal::addInletFloat( float *f ) {
  if ( f ) {
    mInlets.push_back( floatinlet_new( mObject, f ) );
  } else {
    inlet_new( mObject, &mObject->ob_pd, &s_float, &s_float );
    class_addfloat( m_class, ext_float );
    class_addbang( m_class, ext_bang );
  }
}

//------------------------------------------------------------------------------
void RMExternal::cleanup() {
  post("Cleaning up");
  for ( auto i = 1; i < mInlets.size(); i++ ) { inlet_free( mInlets[i] ); }
  for ( auto o : mOutlets ) { outlet_free( o ); }
  mInlets.clear();
  mOutlets.clear();
  exit();
}

//!
//------------------------------------------------------------------------------
t_int *ext_perform( t_int *w ) {
  size_t vIndex = 1;
  
  t_external *x = (t_external *)w[vIndex++];
  auto impl = x->impl;
  
  auto const numIn  = impl->inChannelCount();
  auto const numOut = impl->outChannelCount();
  
  float **const bIn = (float **)alloca(numIn * sizeof(float *));
  for ( auto i = 0; i < numIn; i++ ) {
    bIn[i] = (float *)w[i+vIndex];
    vIndex++;
  }
  
  float **const bOut = (float **)alloca(numOut * sizeof(float *));
  for ( auto i = 0; i < numOut; i++ ) {
    bOut[i] = (float *)w[i+vIndex];
    vIndex++;
  }
  
  impl->process( bIn, bOut, (int)w[vIndex++] /*n*/ );
  
  return (w+vIndex);
}

// #define USE_DEBUG

//------------------------------------------------------------------------------
void ext_dsp( t_external *x, t_signal **sp ) {
  auto impl = x->impl;
#ifndef USE_DEBUG
  auto const ins  = impl->inChannelCount();
  auto const outs = impl->outChannelCount();
  auto const numChannels = ins + outs;
  
  // + 2 for x and sp[0]->s_n (class ptr and vector size)
  auto const vSize = numChannels + 2;
  
  // Add properties to vector.
  // First element is pointer to class
  t_int v[vSize];
  v[0] = (t_int)x;
  
  // Inlets
  size_t vIndex = 1;
  for ( size_t i = 0; i < ins; i++ ) {
    v[vIndex] = (t_int)sp[i]->s_vec;
    vIndex++;
  }
  
  // Signal vector is ordered according to graphical representation of
  // object so first audio outlet will come after all audio inlets
  for ( size_t i = ins; i < numChannels; i++ ) {
    v[vIndex] = (t_int)sp[i]->s_vec;
    vIndex++;
  }
  
  // Add vector size last. All signal vectors of a patch are same size
  v[vIndex] = (t_int)sp[0]->s_n;
  
  dsp_addv(ext_perform, vSize, v);
#else
  dsp_add(ext_perform, 5, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
#endif
}

//------------------------------------------------------------------------------
void ext_free( t_external *x ) {
  delete x->impl;
}
