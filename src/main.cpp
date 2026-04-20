#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Column indexes
// https://www.amsat.org/keplerian-elements-tutorial/
// TODO: use an arena allocator to allocate strings
typedef struct {
  std::string name;
  std::string id;
  std::string epoch; // TODO: parse this date
  double mean_motion;
  double eccentricity;
  double inclination;
  double raan; // Right ascension of the ascending node
  double arg_of_pericenter;
  double mean_anomaly;
  std::string ephemeris_type;
  std::string classification_type;
  int catalogue_id;
  double element_set_number;
  double revolutions_at_epoch;
  double bstar_drag;
  double mean_motion_deriv1;
  double mean_motion_deriv2;
  // The root mean squared error of the orbital fit indicates how well
  // the generated data matches the raw tracking data used to create it.
  double rms_error;
  std::string data_source;
} SatelliteInfo;

int read_satellite_csv(const char *path, std::vector<SatelliteInfo> &infos) {
  std::ifstream file(path);
  if (!file.good() || !file.is_open()) {
    std::cout << "Failed to open " << path << "\n";
    return -1;
  }

  bool first_line = true;
  std::string line = "";
  while (std::getline(file, line)) {
    if (first_line) {
      first_line = false;
      continue;
    }

    SatelliteInfo info;
    size_t current = 0, column = 0;

    while (current < line.length()) {
      size_t next = std::min(line.find(",", current), line.length());
      std::string substr = line.substr(current, next - current);
      current = next + 1;
      column++;

      // clang-format off
      switch (column - 1) {
        case 0: info.name = substr; break;
        case 1: info.id = substr; break;
        case 2: info.epoch = substr; break;
        case 3: info.mean_motion = std::stod(substr); break;
        case 4: info.eccentricity = std::stod(substr); break;
        case 5: info.inclination = std::stod(substr); break;
        case 6: info.raan = std::stod(substr); break;
        case 7: info.arg_of_pericenter = std::stod(substr); break;
        case 8: info.mean_anomaly = std::stod(substr); break;
        case 9: info.ephemeris_type = substr; break;
        case 10: info.classification_type = substr; break;
        case 11: info.catalogue_id = std::stoi(substr); break;
        case 12: info.element_set_number = std::stod(substr); break;
        case 13: info.revolutions_at_epoch = std::stod(substr); break;
        case 14: info.bstar_drag = std::stod(substr); break;
        case 15: info.mean_motion_deriv1 = std::stod(substr); break;
        case 16: info.mean_motion_deriv2 = std::stod(substr); break;
        case 17: info.rms_error = std::stod(substr); break;
        case 18: info.data_source = substr; break;
      };
      // clang-format on
    }

    infos.push_back(info);
  }

  file.close();
  return 0;
}

int main() {
  const char *path = "../data/starlink.csv";
  std::vector<SatelliteInfo> infos;
  read_satellite_csv(path, infos);
  return 0;
}
