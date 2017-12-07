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
//#ifdef PD
#include "m_pd.h"
//#endif
//#ifdef MSP
//#endif

using namespace std;

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
  
  //! TODO: Static create method passing object
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
  int const inChannelCount()  const { return mInChannels;  }
  int const outChannelCount() const { return mOutChannels; }
  
  // Control in/out
  //! Inlets
  class RMInlet;
  using InletRef = std::shared_ptr<RMInlet>;
  
  InletRef   addInletBang  ( string identifier );
  //! Passing optional value pointer creates a passive inlet
  InletRef   addInletFloat ( string identifier, t_float * f = nullptr );
  InletRef   addInletSymbol( string identifier, t_symbol* s = nullptr );
  
  const vector<InletRef>& getInlets() { return mInlets; }
  
  //! Receivers
  virtual void  bangReceived ( InletRef inlet ) {}
  virtual void  floatReceived( InletRef inlet, float value ) {}
  
  //!
  class RMInlet {
    public:
    //!
    template<t_symbol* Type>
    static InletRef create( t_inlet* inlet, string identifier ) {
      return create(inlet, Type, identifier);
    }
    static InletRef create( t_inlet* inlet, t_symbol* type, string identifier );
    ~RMInlet();
    //!
    string      getId()   const { return mId; }
    t_symbol*   getType() const { return mType; }
    protected:
    RMInlet() {};
    string      mId;
    t_symbol*   mType;
    t_inlet*    mInlet;
  };
  
protected:
  
private:
  void    cleanup();
  int     mInChannels;
  int     mOutChannels;
  std::vector<InletRef>  mInlets;
  std::vector<t_outlet *> mOutlets;
};


// Forward declarations
void  ext_dsp( t_external *x, t_signal **sp );
void *ext_new( t_symbol *s, int argc, t_atom *argv );

//! Bang receivers
typedef void (*t_bangfunc) (t_external *);
#define addBangFunc(num) \
void ext_bangin_##num( t_external *x ) { \
  auto impl = x->impl; \
  impl->bangReceived( impl->getInlets()[num-1] ); \
}
addBangFunc(1)
addBangFunc(2)
addBangFunc(3)
addBangFunc(4)
addBangFunc(5)
static vector<t_bangfunc> bangfuncs = { ext_bangin_1, ext_bangin_2, ext_bangin_3, ext_bangin_4, ext_bangin_5 };

//! Float receivers
typedef void (*t_floatfunc)(t_external *, float);
#define addFloatFunc(num) \
void ext_floatin_##num( t_external *x, float f ) { \
  auto impl = x->impl; \
  impl->floatReceived( impl->getInlets()[num-1], f ); \
}
addFloatFunc(1)
addFloatFunc(2)
addFloatFunc(3)
addFloatFunc(4)
addFloatFunc(5)
static vector<t_floatfunc> floatfuncs = { ext_floatin_1, ext_floatin_2, ext_floatin_3, ext_floatin_4, ext_floatin_5 };


//!
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
    auto signal = &s_signal;
    auto it = inlet_new( mObject, &mObject->ob_pd, signal, signal );
    mInlets.push_back( RMInlet::create( it, signal, "SignalIn " + std::to_string(i+1) ) );
  }
  
  for ( auto i = 0; i < outChannels; i++ ) {
    mOutlets.push_back( outlet_new( mObject, &s_signal ) );
  }
  
  mInChannels  = inChannels;
  mOutChannels = outChannels;
}

//------------------------------------------------------------------------------
RMExternal::InletRef RMExternal::addInletBang( string identifier ) {
  auto idx    = mInlets.size();
  auto symbol = gensym(("ext_bangin_" + to_string(idx+1)).c_str());
  auto func   = (t_method)bangfuncs[idx];
  class_addmethod( m_class, func, symbol, A_NULL );
  auto bang = &s_bang;
  auto inlet = inlet_new( mObject, &mObject->ob_pd, bang, symbol );
  mInlets.push_back( RMInlet::create( inlet, bang, identifier ) );
  return mInlets.back();
}

//------------------------------------------------------------------------------
RMExternal::InletRef RMExternal::addInletFloat( string identifier, float *f ) {
  t_inlet* it = nullptr;
  if ( f ) {
    it = floatinlet_new( mObject, f );
    return nullptr;
  } else {
    auto idx    = mInlets.size();
    auto symbol = gensym(("ext_floatin_" + to_string(idx+1)).c_str());
    auto func   = (t_method)floatfuncs[idx];
    class_addmethod( m_class, func, symbol, A_FLOAT, A_NULL );
    it = inlet_new( mObject, &mObject->ob_pd, &s_float, symbol );
  }
  mInlets.push_back( RMInlet::create( it, &s_float, identifier ) );
  return mInlets.back();
}

//------------------------------------------------------------------------------
void RMExternal::cleanup() {
  post("Cleaning up");
  for ( auto o : mOutlets ) { outlet_free( o ); }
  mInlets.clear();
  mOutlets.clear();
  exit();
}

//! RMInlet
//------------------------------------------------------------------------------
RMExternal::InletRef RMExternal::RMInlet::create( t_inlet* inlet, t_symbol* type, string identifier ) {
  auto i = new RMInlet;
  i->mInlet = inlet;
  i->mType  = type;
  i->mId    = identifier;
  return InletRef( i );
}

//------------------------------------------------------------------------------
RMExternal::RMInlet::~RMInlet() {
  post("Deleting inlet");
  inlet_free( mInlet );
}

//! Pd
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
