

// An End of a track segment can be connected to another
struct End {
  x: f32,
  y: f32,
  angle: f32,
  Option<Rc<Connection>>,
}

// Represents a geometrically simple section of track
enum Track {
  Straight(End, End),
  Curve(End, End, f32),
  Transition(End, End, f32, f32),
}
