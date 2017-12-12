//
//  TRextern.h
//  TRextern
//
//  Created by Ragnar Hrafnkelsson on 01/12/2017.
//  Copyright © 2017 Reactify. All rights reserved.
//

#pragma once

#include <vector>
#include <string>
#ifdef PD
#include "m_pd.h"
#else
#include "ext.h"
#include "z_dsp.h"
#endif

using namespace std;

//! Pointer to class
static t_class* m_class;

using InletRef  = std::shared_ptr<class Inlet>;
using OutletRef = std::shared_ptr<class Outlet>;

//! Base external object
class TRextern {
public:
  TRextern();
  virtual ~TRextern();
  
  //! Override to perfom setup
  virtual void  setup( int argc, t_atom *argv ) {}
  //! Override to free resources before exit
  virtual void  exit() {}
  //! Override to process audio
  virtual void  process( t_sample **const inChannels, t_sample **const outChannels, long size ) {};
  //! Override to receive control values from inlets
  virtual void  bangReceived  ( InletRef inlet ) {}
  virtual void  intReceived   ( InletRef inlet, long value ) {}
  virtual void  floatReceived ( InletRef inlet, t_sample value ) {}
  virtual void  symbolReceived( InletRef inlet, t_symbol* symbol ) {}

  // Audio in/out
  virtual void  setupIO( int inChannels, int outChannels ) final;
  
  int const inChannelCount()  const { return mInChannels;  }
  int const outChannelCount() const { return mOutChannels; }
  
  //! Control in/out
  InletRef    addInletBang  ( string identifier );
  //! Passing optional value pointer creates a passive inlet
  //  which won't pass changes on to receivers below. TODO: passive inlets for Max?
  InletRef    addInletFloat ( string identifier, t_sample *f = nullptr );
  InletRef    addInletSymbol( string identifier, t_symbol *s = nullptr );
  
  OutletRef   addOutlet( string identifier );
  
  const vector<InletRef>&  getInlets()  { return mInlets; }
  const vector<OutletRef>& getOutlets() { return mOutlets; }
  
  // Do not call. Used internally
  virtual void layoutInOuts() final;
  
#ifdef PD
  t_object*   mObject; // Pointer to internal object-properties. Do not use.
#else
  t_pxobject* mObject; // Pointer to internal object-properties. Do not use.
#endif
  
  struct _external* mParent;
  
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
#ifndef PD
typedef void t_inlet;
typedef void t_outlet;
#endif

struct NonCopyable {
  NonCopyable & operator=(const NonCopyable&) = delete;
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable() = default;
};

class Inlet : NonCopyable {
  friend TRextern;
public:
  ~Inlet();
  const string&   getId()   const { return mId; }
  t_symbol*       getType() const { return mType; }
protected:
  //! Meant for internal instantation only
  static InletRef create( t_inlet* inlet, t_symbol* type, string identifier );
  t_inlet*   mInlet;
private:
  Inlet()    {};
  string     mId;
  t_symbol*  mType;
};

class Outlet : NonCopyable {
  friend TRextern;
public:
  ~Outlet();
  const string  getId();
  t_symbol*     getType() const { return mType; }
  void          sendBang();
  void          sendFloat( t_sample f );
  void          sendSymbol( t_symbol *s );
  //void          sendList( t_symbol *s );
  bool          isSignal();
protected:
  //! Meant for internal instantation only
  static OutletRef create( t_outlet* outlet, t_symbol* type, string identifier );
  t_outlet*    mOutlet;
private:
  Outlet()   {};
  string     mId;
  t_symbol*  mType;
};

//! Class dataspace
typedef struct _external {
#ifdef PD
  t_object    x_obj; // Internal object-properties
#else
  t_pxobject  x_obj;
  long        m_in;  // space for the inlet number used by all the proxies
#endif
  TRextern*   impl;
} t_external;


// Forward declarations and class methods
void *ext_new( t_symbol *s, int argc, t_atom *argv );
#ifdef PD
void  ext_dsp( t_external *x, t_signal **sp );

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
typedef void (*t_floatfunc)(t_external *, t_sample);
#define addFloatFunc(num) \
void ext_floatin_##num( t_external *x, t_sample f ) { \
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

#else // Max

InletRef inletFromProxy( t_external *x ) {
  auto idx = proxy_getinlet((t_object *)x);
  return x->impl->getInlets()[idx];
}

void ext_bangin( t_external *x ) {
  auto it = inletFromProxy(x);
  if ( it->getType() == gensym("bang") ) {
    x->impl->bangReceived( it );
  } else {
    post("Inlet expects %s", it->getType()->s_name);
  }
}

void ext_floatin( t_external *x, t_sample value ) {
  auto it = inletFromProxy(x);
  if ( it->getType() == gensym("float") ) {
    x->impl->floatReceived( it, value );
  } else {
    post("Inlet expects %s", it->getType()->s_name);
  }
}

void ext_intin( t_external *x, long value ) {
  auto it = inletFromProxy(x);
  if ( it->getType() == gensym("int") ) {
    x->impl->intReceived( it, value );
  } else {
    post("Inlet expects %s", it->getType()->s_name);
  }
}

void ext_symbolin( t_external *x, t_symbol *s ) {
  auto it = inletFromProxy(x);
  if ( it->getType() == gensym("symbol") ) {
    x->impl->symbolReceived( it, s );
  } else {
    post("Inlet expects %s", it->getType()->s_name);
  }
}

#endif


//! TRextern Implmentation

//------------------------------------------------------------------------------
TRextern::TRextern() : mInChannels(0), mOutChannels(0) {}

//------------------------------------------------------------------------------
TRextern::~TRextern() {
  cleanup();
}

//------------------------------------------------------------------------------
void TRextern::setupIO( int inChannels, int outChannels ) {
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
InletRef TRextern::addInletBang( string identifier ) {
  t_inlet* it = nullptr;
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
InletRef TRextern::addInletFloat( string identifier, t_sample *f ) {
  t_inlet* it = nullptr;
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
InletRef TRextern::addInletSymbol( string identifier, t_symbol* s ) {
  t_inlet* it = nullptr;
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
InletRef TRextern::addInletSignal( string identifier ) {
  t_inlet* it = nullptr;
  auto signal = gensym("signal");
#ifdef PD
  it = inlet_new( mObject, &mObject->ob_pd, signal, signal );
#else
  // Max audio inlets are empty placeholders
  // Audio inlets are created by calling dsp_setup( mObject, inChannels );
#endif
  mInlets.push_back( Inlet::create( it, signal, identifier ) );
  return mInlets.back();
}

//! Outlets
//------------------------------------------------------------------------------
OutletRef TRextern::addOutlet( string identifier ) {
  t_outlet* ot = nullptr;
#ifdef PD
  ot = outlet_new( mObject, gensym( identifier.c_str()) );
#endif
  mOutlets.push_back( Outlet::create( ot, gensym("control"), identifier ) );
  return mOutlets.back();
}

//------------------------------------------------------------------------------
OutletRef TRextern::addOutletSignal( string identifier ) {
  t_outlet* ot = nullptr;
#ifdef PD
  ot = outlet_new( mObject, &s_signal );
#endif
  mOutlets.push_back( Outlet::create( ot, gensym("signal"), identifier ) );
  return mOutlets.back();
}

// Max inlet ordering is done from right to left so we need to
// loop through and create in right order after initial setup
void TRextern::layoutInOuts() {
#ifndef PD
  // Audio inlets are already constructed here so we exclude from loop
  for ( auto i = mInlets.size(); i-- > inChannelCount() ; ) {
    auto it = mInlets[i];
    it->mInlet = proxy_new( mParent, i, &mParent->m_in );
  }
  
  for ( auto i = mOutlets.size(); i-- > 0 ; ) {
    auto ot = mOutlets[i];
    ot->mOutlet = outlet_new( mParent, ot->getType()->s_name );
  }
#endif
}

//------------------------------------------------------------------------------
void TRextern::cleanup() {
  post("Cleaning up");
  mInlets.clear();
  mOutlets.clear();
  exit();
}

//! Inlet
//------------------------------------------------------------------------------
InletRef Inlet::create( t_inlet* inlet, t_symbol* type, string identifier ) {
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
OutletRef Outlet::create( t_outlet* outlet, t_symbol* type, string identifier ) {
  auto i = new Outlet;
  i->mOutlet = outlet;
  i->mId     = identifier;
  i->mType   = type;
  return OutletRef( i );
}

//------------------------------------------------------------------------------
Outlet::~Outlet() {
  post("Deleting outlet");
#ifdef PD
  outlet_free( mOutlet );
#else
  outlet_delete( mOutlet );
#endif
}

const string Outlet::getId() {
#ifdef PD
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
void Outlet::sendFloat( t_sample f ) {
  outlet_float(mOutlet, f);
}

//------------------------------------------------------------------------------
void Outlet::sendSymbol( t_symbol *s ) {
#ifdef PD
  outlet_symbol(mOutlet, s);
#else
#warning TODO MAX
#endif
}

bool Outlet::isSignal() {
  return mType == gensym("signal");
}

#ifdef PD
//! DSP routines
//------------------------------------------------------------------------------
t_int *ext_perform( t_int *w ) {
  size_t vIndex = 1;
  
  auto impl = ((t_external *)w[vIndex++])->impl;
  
  auto const numIn  = impl->inChannelCount();
  auto const numOut = impl->outChannelCount();
  
  t_sample **const bIn = (t_sample **)alloca(numIn * sizeof(t_sample *));
  for ( auto i = 0; i < numIn; i++ ) {
    bIn[i] = (t_sample *)w[i+vIndex];
    vIndex++;
  }
  
  t_sample **const bOut = (t_sample **)alloca(numOut * sizeof(t_sample *));
  for ( auto i = 0; i < numOut; i++ ) {
    bOut[i] = (t_sample *)w[i+vIndex];
    vIndex++;
  }
  
  impl->process( bIn, bOut, (long)w[vIndex++] /*n*/ );
  
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
  auto vIndex = 1;
  for ( auto i = 0; i < ins; i++ ) {
    v[vIndex] = (t_int)sp[i]->s_vec;
    vIndex++;
  }
  
  // Signal vector is ordered according to graphical representation of
  // object so first audio outlet will come after all audio inlets
  for ( auto i = ins; i < numChannels; i++ ) {
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

#else // Max

//------------------------------------------------------------------------------
void ext_perform64(t_external *x, t_object *dsp64, t_sample **ins, long numins, t_sample **outs, long numouts, long sampleframes, long flags, void *userparam)
{
  x->impl->process( ins, outs, sampleframes );
}

//------------------------------------------------------------------------------
void ext_dsp64(t_external *x, t_object *dsp64, short *count, t_sample samplerate, long maxvectorsize, long flags)
{
  object_method(dsp64, gensym("dsp_add64"), x, ext_perform64, 0, NULL);
}

#endif

//------------------------------------------------------------------------------
t_external *ext_alloc() {
#ifdef PD
  return (t_external *)pd_new(m_class);
#else
  return (t_external *)object_alloc(m_class);
#endif
}

//------------------------------------------------------------------------------
void ext_free( t_external *x ) {
#ifndef PD
  dsp_free((t_pxobject *)x);
#endif
  delete x->impl;
}

//------------------------------------------------------------------------------
void tr_initialise( const char* title ) {
#ifdef PD
  m_class = class_new(gensym(title),
                      (t_newmethod)ext_new,
                      (t_method)ext_free,
                      sizeof(t_external),
                      CLASS_NOINLET,
                      A_GIMME,
                      A_NULL);
#else
  m_class = class_new(title,
                      (method)ext_new,
                      (method)ext_free,
                      sizeof(t_external),
                      0L,
                      A_GIMME,
                      0);
  class_addmethod(m_class, (method)ext_bangin,  "bang", 0);
  class_addmethod(m_class, (method)ext_floatin, "float",  A_FLOAT, 0);
  class_addmethod(m_class, (method)ext_intin,   "int",    A_LONG, 0);
  class_addmethod(m_class, (method)ext_symbolin,"symbol", A_SYM, 0);
  //  class_addmethod(m_class, (method)ext_list,     "list", A_GIMME, 0);
  //  class_addmethod(m_class, (method)ext_anything, "anything", A_GIMME, 0);
#warning TODO MAX
  // TODO: Support for non DSP objects
  // If dsp
  class_addmethod(m_class, (method)ext_dsp64, "dsp64",  A_CANT, 0);
  class_dspinit(m_class);
  // endif
  class_register(CLASS_BOX, m_class);
#endif
}

// Replaces occurences of '_tilde' with ~
const char* tr_tildefy( string title ) {
  static string tilde = "_tilde";
  return title.replace( title.find(tilde), tilde.length(), "~" ).c_str();
}

#define TREXTERN_CREATE( CLASS ) \
\
void *ext_new( t_symbol *s, int argc, t_atom *argv ) { \
  t_external *x = ext_alloc(); \
  x->impl = new CLASS; \
  x->impl->mObject = &x->x_obj; \
  x->impl->mParent = x; \
  x->impl->setup(argc, argv); \
  x->impl->layoutInOuts(); \
  return (x); \
} \
\
extern "C" __attribute__((visibility("default"))) \
void pan_tilde_setup(void) { \
  tr_initialise(tr_tildefy(#CLASS)); \
} \
\
void ext_main(void *r) { \
  tr_initialise(tr_tildefy(#CLASS)); \
} \
