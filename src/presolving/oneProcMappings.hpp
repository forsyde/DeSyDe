#ifndef __ONEPROCMODEL__
#define __ONEPROCMODEL__

#include <math.h>
#include <vector>
#include <tuple>

#include <gecode/int.hh>
#include <gecode/gist.hh>
#include <gecode/minimodel.hh>


#include "../applications/applications.hpp"
#include "../platform/platform.hpp"
#include "../system/mapping.hpp"
#include "../systemDesign/designDecisions.hpp"
#include "../settings/dse_settings.hpp"

using namespace Gecode;

/**
 * Model as part of presolving: Find all possible mappings where applications are isolated on
 * processors (meaning: no communication on interconnect), because for these cases no exploration
 * with different schedules need to be done. A simple check that at least one valid schedule exists
 * is sufficient.
 */
class OneProcModel : public Space {
  
private:
  Applications*            apps;         /**< Pointer to the application object. */
  Platform*                platform;    /**< Pointer to the platform object. */
  Mapping*                 mapping;    /**< Pointer to the mapping object. */
  DesignDecisions*         desDec;        /**< Pointer to the design decision object. */
  Config&                  settings;    /**< Pointer to the setting object. */
  
  //DECISION VARIABLES
  //mapping of firings onto processors
  IntVarArray proc;
  //for "flexible" procs with modes: economy, regular and performance
  IntVarArray proc_mode;
  
  
public:

  OneProcModel(Mapping* p_mapping, Config& cfg);
  
  OneProcModel(bool share, OneProcModel& s);
  
  // Copies the space
  virtual Space* copy(bool share);
  
  // Prints the variables
  void print(std::ostream& out) const;

  virtual void constrain(const Space& _b){
  }
  
  vector<tuple<int,int>> getResult() const;
  

  
};

#endif
