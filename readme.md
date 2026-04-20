# Low earth orbit visualizer

In C++ with OpenGL, everything from scratch.

- Read CSV data to get satellite info

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

- The eccentricty of an ellipse is the distance between the two foci
  [-sqrt(a^2 - b^2), -sqrt(a^2 - b^2)], divided by the semi major axis.
- The right ascension of the ascension node (Ω) defines the orientation of an object's orbit in space

- Periapsis: Point in an orbit closest to the center of mass of a system.
- Apoapsis: Point in an orbit furthest from the center of mass of a system.
  - Perigee and apogee are terms used specifically for Earth.

- Orbital node: When an orbit intersects a plane of reference to which it's inclined.
  - Ascending node: When the orbit intersects a plane of reference moving north.
  - Descending node: When the orbit intersects a plane of reference moving south.

- Low earth orbit: Orbit altitude of <= 2 km.
- Drag: An opposing force exterted by the Earth's atmosphere on objects orbiting in LEO.
- SGP4: Simplified general pertubations 4
- TLE: Two-line element set. Although it's the de facto standard, it's very outdated (Y2K problems, running out of satellite identifiers, etc), so instead we'll be parsing CSV

---

- [Basics of Spaceflight](https://science.nasa.gov/learn/basics-of-space-flight/)
- [Planetary Orbits](https://science.nasa.gov/learn/basics-of-space-flight/chapter5-1/)
- [Supplemental GP Element Sets](https://celestrak.org/NORAD/elements/supplemental/index.php?FORMAT=csv)
- [United States Space Command](https://en.wikipedia.org/wiki/United_States_Space_Command)
- [United States Space Surveillance Network](https://en.wikipedia.org/wiki/United_States_Space_Surveillance_Network)
- [Satellite drag](https://www.swpc.noaa.gov/impacts/satellite-drag)
- [SPACETRACK REPORT NO. 3](https://celestrak.org/NORAD/documentation/spacetrk.pdf)
- [Revisiting Spacetrack Report #3: Rev 2](https://celestrak.org/publications/AIAA/2006-6753/AIAA-2006-6753-Rev2.pdf)
- [Two-line element set](https://en.wikipedia.org/wiki/Two-line_element_sete
- [A New Way to Obtain GP Data (aka TLEs)](https://celestrak.org/NORAD/documentation/gp-data-formats.php)
- [What are the ascending and descending nodes of an orbit?](https://www.youtube.com/watch?v=5WB93RWXpns)
