# Low earth orbit visualizer

In C++ with OpenGL, everything from scratch.

- Read TLE data to get satellite and other bodies in low earth orbit
    - https://celestrak.org/NORAD/documentation/tle-fmt.php

- Implement the SGP4 algorithm to predict their motions
    - Test and benchmark against existing implementations:

- Do all of this optimally for all the satellites in real time

- Visualize them moving around the earth
   - Render the Earth realistically (wrap a heightmap around a sphere and texture correctly)

   - Show the trajectories of the satellites

   - Click satellites to show info on them

   - Zoom camera in/out, orbit around the earth

- Add a button to zoom in on your GPS position and view the satellites overhead

---

- Eccentricity: Deviation of an orbital path from a perfect circle.
- Perigee: Farthest point in an elliptical orbit around the earth.
- Apogee: Closest point in an elliptical orbit around the earth.
- Low earth orbit: Orbit altitude of <= 2 km.
- Drag: An opposing force exterted by the Earth's atmosphere on objects orbiting in LEO.
- SGP4: Simplified general pertubations 4

---

[United States Space Command](https://en.wikipedia.org/wiki/United_States_Space_Command)
[United States Space Surveillance Network](https://en.wikipedia.org/wiki/United_States_Space_Surveillance_Network)
[Satellite drag](https://www.swpc.noaa.gov/impacts/satellite-drag)
[SPACETRACK REPORT NO. 3](https://celestrak.org/NORAD/documentation/spacetrk.pdf)
[Revisiting Spacetrack Report #3: Rev 2](https://celestrak.org/publications/AIAA/2006-6753/AIAA-2006-6753-Rev2.pdf)
[NORAD Two-Line Element Set Format](https://celestrak.org/NORAD/documentation/tle-fmt.php)
