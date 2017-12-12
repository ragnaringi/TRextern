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
#ifdef PD
#include "m_pd.h"
#else
#include "ext.h"
#include "z_dsp.h"
#endif

#ifdef PD
typedef float  trfloat;
#else
typedef double trfloat;
#endif

using namespace std;

//! Pointer to class
static t_class* m_class;

struct RMExternal;
class Inlet;
using InletRef  = std::shared_ptr<Inlet>;
class Outlet;
using OutletRef = std::shared_ptr<Outlet>;

//! Class dataspace
typedef struct _external {
#ifdef PD
  t_object    x_obj; // Internal object-properties
#else
  t_pxobject  x_obj;
  long        m_in;    // space for the inlet number used by all the proxies
#endif
  RMExternal*   impl;
} t_external;


//! Base external object
struct RMExternal {
  public:
#ifdef PD
  t_object* mObject;   // Pointer to internal object-properties. Do not use.
#else
  t_pxobject* mObject; // Pointer to internal object-properties. Do not use.
#endif
  
  t_external* mParent;
  
  //! TODO: Static create method passing object
  RMExternal();
  virtual ~RMExternal();
  
  //! Override to perfom setup
  virtual void  setup( int argc, t_atom *argv ) {}
  //! Override to free resources before exit
  virtual void  exit() {}
  //! Override to process audio
  virtual void  process( float ** inChannels,  float ** outChannels,  int size ) {};
  virtual void  process( double ** inChannels, double ** outChannels, int size ) {};

  // Setup audio inlets and outlets
  virtual void  setupIO( int inChannels, int outChannels ) final;
  
  // Get number of audio ins/outs
  int const inChannelCount()  const { return mInChannels;  }
  int const outChannelCount() const { return mOutChannels; }
  
  virtual void layoutInlets() final;
  
  //! Control in/out
  InletRef   addInletBang  ( string identifier );
  //! Passing optional value pointer creates a passive inlet
  //  which won't pass changes on to receivers below
  InletRef   addInletFloat ( string identifier, float  * f = nullptr );
  InletRef   addInletSymbol( string identifier, t_symbol * s = nullptr );
  
  const vector<InletRef>& getInlets() { return mInlets; }
  
  //! Receivers
  virtual void  bangReceived  ( InletRef inlet ) {}
  virtual void  intReceived   ( InletRef inlet, long value ) {}
  virtual void  floatReceived ( InletRef inlet, trfloat value ) {}
  virtual void  symbolReceived( InletRef inlet, t_symbol* symbol ) {}
  
  OutletRef   addOutlet( string identifier );
  
  const vector<OutletRef>& getOutlets() { return mOutlets; }
  
protected:
  
private:
  //!
  InletRef   addInletSignal ( string identifier );
  OutletRef  addOutletSignal( string identifier );
  
  void    cleanup();
  int     mInChannels;
  int     mOutChannels;
  vector<InletRef>  mInlets;
  vector<OutletRef> mOutlets;
};

//!
class Inlet {
  friend RMExternal;
public:
  ~Inlet();
  const string&   getId()   const { return mId; }
  t_symbol*       getType() const { return mType; }
protected:
  //! Not meant for instantation outside of main class
  static InletRef create( void* inlet, t_symbol* type, string identifier );
  void*  mInlet;
private:
  Inlet()    {};
  string     mId;
  t_symbol*  mType;
};

//!
class Outlet {
  friend RMExternal;
public:
  ~Outlet();
  const string  getId();
  void          sendBang();
  void          sendFloat( float f );
  void          sendSymbol( t_symbol *s );
  //void          sendList( t_symbol *s );
  bool          isSignal();
protected:
  //! Not meant for instantation outside of main class
  static OutletRef create( void* outlet, t_symbol* type, string identifier );
private:
  Outlet()   {};
  string     mId;
  void*      mOutlet;
  t_symbol*  mType;
};


// Forward declarations
#ifdef PD
void  ext_dsp( t_external *x, t_signal **sp );
void *ext_new( t_symbol *s, int argc, t_atom *argv );
#endif

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
addBangFunc(6)
addBangFunc(7)
static vector<t_bangfunc> bangfuncs = {
  ext_bangin_1,
  ext_bangin_2,
  ext_bangin_3,
  ext_bangin_4,
  ext_bangin_5,
  ext_bangin_6,
  ext_bangin_7
};

//! Float receivers
typedef void (*t_floatfunc)(t_external *, trfloat);
#define addFloatFunc(num) \
void ext_floatin_##num( t_external *x, trfloat f ) { \
  auto impl = x->impl; \
  impl->floatReceived( impl->getInlets()[num-1], f ); \
}
addFloatFunc(1)
addFloatFunc(2)
addFloatFunc(3)
addFloatFunc(4)
addFloatFunc(5)
addFloatFunc(6)
addFloatFunc(7)
static vector<t_floatfunc> floatfuncs = {
  ext_floatin_1,
  ext_floatin_2,
  ext_floatin_3,
  ext_floatin_4,
  ext_floatin_5,
  ext_floatin_6,
  ext_floatin_7
};

//! Symbol receivers
typedef void (*t_symbolfunc)(t_external *, t_symbol*);
#define addSymbolFunc(num) \
void ext_symbolin_##num( t_external *x, t_symbol* s ) { \
  auto impl = x->impl; \
  impl->symbolReceived( impl->getInlets()[num-1], s ); \
}
addSymbolFunc(1)
addSymbolFunc(2)
addSymbolFunc(3)
addSymbolFunc(4)
addSymbolFunc(5)
addSymbolFunc(6)
addSymbolFunc(7)
static vector<t_symbolfunc> symbolfuncs = {
  ext_symbolin_1,
  ext_symbolin_2,
  ext_symbolin_3,
  ext_symbolin_4,
  ext_symbolin_5,
  ext_symbolin_6,
  ext_symbolin_7
};
#ifdef PD
#else // Max
void ext_dsp64(t_external *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags);
void ext_perform64(t_external *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam);
#endif


//!
//------------------------------------------------------------------------------
RMExternal::RMExternal() : mInChannels(0), mOutChannels(0) {}

//------------------------------------------------------------------------------
RMExternal::~RMExternal() {
  cleanup();
}

//------------------------------------------------------------------------------
void RMExternal::setupIO( int inChannels, int outChannels ) {
#ifdef PD
  class_addmethod( m_class, (t_method)ext_dsp, gensym("dsp"), A_NULL );
#else
  dsp_setup( mObject, inChannels );
#endif
  
  for ( auto i = 0; i < inChannels; i++ ) {
    addInletSignal( "SignalIn " + std::to_string(i+1) );
  }
  
  for ( auto i = 0; i < outChannels; i++ ) {
    addOutletSignal( "SignalOut " + std::to_string(i+1) );
  }
  
  mInChannels  = inChannels;
  mOutChannels = outChannels;
}

//------------------------------------------------------------------------------
InletRef RMExternal::addInletBang( string identifier ) {
  void* it = nullptr;
#ifdef PD
  auto idx    = mInlets.size();
  auto symbol = gensym(("ext_bangin_" + to_string(idx+1)).c_str());
  auto func   = (t_method)bangfuncs[idx];
  class_addmethod( m_class, func, symbol, A_NULL );
  it = inlet_new( mObject, &mObject->ob_pd, &s_bang, symbol );
#endif
  mInlets.push_back( Inlet::create( it, gensym("bang"), identifier ) );
  return mInlets.back();
}

//------------------------------------------------------------------------------
InletRef RMExternal::addInletFloat( string identifier, float *f ) {
  void* it = nullptr;
#ifdef PD
  if ( f ) {
    it = floatinlet_new( mObject, f );
  } else {
    auto idx    = mInlets.size();
    auto symbol = gensym(("ext_floatin_" + to_string(idx+1)).c_str());
    auto func   = (t_method)floatfuncs[idx];
    class_addmethod( m_class, func, symbol, A_FLOAT, A_NULL );
    it = inlet_new( mObject, &mObject->ob_pd, &s_float, symbol );
  }
#endif
  mInlets.push_back( Inlet::create( it, gensym("float"), identifier ) );
  return mInlets.back();
}

//------------------------------------------------------------------------------
InletRef RMExternal::addInletSymbol( string identifier, t_symbol* s ) {
  void* it = nullptr;
#ifdef PD
  if ( s ) {
    it = symbolinlet_new( mObject, &s );
  } else {
    auto idx    = mInlets.size();
    auto symbol = gensym(("ext_symbolin_" + to_string(idx+1)).c_str());
    auto func   = (t_method)symbolfuncs[idx];
    class_addmethod( m_class, func, symbol, A_SYMBOL, A_NULL );
    it = inlet_new( mObject, &mObject->ob_pd, &s_symbol, symbol );
  }
#endif
  mInlets.push_back( Inlet::create( it, gensym("symbol"), identifier ) );
  return mInlets.back();
}

//------------------------------------------------------------------------------
InletRef RMExternal::addInletSignal( string identifier ) {
  void* it = nullptr;
#ifdef PD
  auto signal = &s_signal;
  it = inlet_new( mObject, &mObject->ob_pd, signal, signal );
  mInlets.push_back( Inlet::create( it, signal, identifier ) );
#else
  // Max audio inlets are empty placeholders
  // Audio inlets are created by calling dsp_setup( mObject, inChannels );
  mInlets.push_back( Inlet::create( it, gensym("signal"), identifier ) );
#endif
  return mInlets.back();
}

//! Outlets
//------------------------------------------------------------------------------
OutletRef RMExternal::addOutlet( string identifier ) {
#ifdef PD
  auto ot = outlet_new( mObject, gensym(identifier.c_str()) );
#else
  auto ot = outlet_new( mObject, identifier.c_str() );
#endif
  mOutlets.push_back( Outlet::create( ot, nullptr, identifier ) );
  return mOutlets.back();
}

//------------------------------------------------------------------------------
OutletRef RMExternal::addOutletSignal( string identifier ) {
#ifdef PD
  auto ot = outlet_new( mObject, &s_signal );
#else
  auto ot = outlet_new( mParent, "signal" );
#endif
  mOutlets.push_back( Outlet::create( ot, gensym("signal"), identifier ) );
  return mOutlets.back();
}

void RMExternal::layoutInlets() { // For Max reordering of inlets
#ifndef PD
  auto numproxies = mInlets.size() - 1;
  for ( auto i = numproxies; 0 < i; i-- ) {
    auto it = mInlets[i];
    it->mInlet = proxy_new( mParent, i, &mParent->m_in );
  }
#endif
}

//------------------------------------------------------------------------------
void RMExternal::cleanup() {
  post("Cleaning up");
  mInlets.clear();
  mOutlets.clear();
  exit();
}

//! Inlet
//------------------------------------------------------------------------------
InletRef Inlet::create( void* inlet, t_symbol* type, string identifier ) {
  auto i = new Inlet;
  i->mInlet = inlet;
  i->mType  = type;
  i->mId    = identifier;
  return InletRef( i );
}

//------------------------------------------------------------------------------
Inlet::~Inlet() {
  post("Deleting inlet");
  if ( mInlet ) {
#ifdef PD
    inlet_free( mInlet );
#else
    proxy_delete( mInlet );
#endif
  }
}

//! Outlet
//------------------------------------------------------------------------------
OutletRef Outlet::create( void* outlet, t_symbol* type, string identifier ) {
  auto i = new Outlet;
  i->mOutlet = outlet;
  i->mId     = identifier;
  i->mType   = type;
  return OutletRef( i );
}

//------------------------------------------------------------------------------
Outlet::~Outlet() {
  post("Deleting outlet");
#if PD
  outlet_free( mOutlet );
#else
  outlet_delete( mOutlet );
#endif
}

const string Outlet::getId() {
#if PD
  return string(outlet_getsymbol(mOutlet)->s_name);
#else
  return mId;
#endif
}

//------------------------------------------------------------------------------
void Outlet::sendBang() {
  outlet_bang(mOutlet);
}

//------------------------------------------------------------------------------
void Outlet::sendFloat(float f) {
  outlet_float(mOutlet, f);
}

//------------------------------------------------------------------------------
void Outlet::sendSymbol( t_symbol *s ) {
#if PD
  outlet_symbol(mOutlet, s);
#else
#warning TODO MAX
#endif
}

bool Outlet::isSignal() {
  return mType == gensym("signal");
}

#ifdef PD
//! Pd
//------------------------------------------------------------------------------
t_int *ext_perform( t_int *w ) {
  size_t vIndex = 1;
  
  auto impl = ((t_external *)w[vIndex++])->impl;
  
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

#else

//------------------------------------------------------------------------------
void ext_perform64(t_external *x, t_object *dsp64, double **ins, long numins, double **outs, long numouts, long sampleframes, long flags, void *userparam)
{
  x->impl->process( ins, outs, sampleframes );
}

//------------------------------------------------------------------------------
void ext_dsp64(t_external *x, t_object *dsp64, short *count, double samplerate, long maxvectorsize, long flags)
{
  object_method(dsp64, gensym("dsp_add64"), x, ext_perform64, 0, NULL);
}

#endif
