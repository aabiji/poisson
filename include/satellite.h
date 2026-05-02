#pragma once

#include <perturb/perturb.hpp>
#include <string>
#include <vector>

#include "mesh.h"

struct Satellite {
  std::string name;
  std::string id;
  perturb::Satellite model;
};

class Constellation {
public:
  void set_current_time();
  int load_satellite_data(std::string csv_path);
  void propagate(double step_seconds, double earth_scale,
                 std::vector<InstanceData> &instances);

private:
  perturb::JulianDate date;
  std::vector<Satellite> satellites;
};
