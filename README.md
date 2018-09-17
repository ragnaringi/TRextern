# TRextern
A tiny framework for building max and pure data externals in a single project.

## Instructions
Run `generate.py` to create new projects, e.g. `python generate.py Name -o Output/Dir`. The single `*.cpp` file in the resulting directory is your external base class. Signal [~] objects should be named `NAME_tilde`.

### Example output
```
class hello_world : public TRextern {
public:
  void  setup( int argc, t_atom *argv ) override;
  void  exit() override;
  
  void  bangReceived  ( InletRef inlet ) override;
  void  floatReceived ( InletRef inlet, t_sample value ) override;
  void  symbolReceived( InletRef inlet, t_symbol* symbol ) override;
};

//------------------------------------------------------------------------------
void hello_world::setup( int argc, t_atom *argv ) {
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
  
  std::cout << "Hello World" << std::endl;
}

//------------------------------------------------------------------------------
void hello_world::exit() {
  // Free any resources
}

//------------------------------------------------------------------------------
void hello_world::bangReceived( InletRef inlet ) {
  post("Bang received from: %s", inlet->getId().c_str());
}

//------------------------------------------------------------------------------
void hello_world::floatReceived( InletRef inlet, t_sample value ) {
  post("Float %f received from: %s", value, inlet->getId().c_str());
}

//------------------------------------------------------------------------------
void hello_world::symbolReceived( InletRef inlet, t_symbol* symbol ) {
  post("Symbol %s received from %s", symbol->s_name, inlet->getId().c_str());
}
```

### TODO
Windows support. Tested on MacOS (Pd/Max) and Linux (Pd).
