FUNCTION
Mat4 mat4_identity() {
  return {
    1,0,0,0,
      0,1,0,0,
      0,0,1,0,
      0,0,0,1,
  };
}

FUNCTION
Mat4 mat4_scale(Vec3 a) {
  return {
    a.x, 0, 0, 0,
      0, a.y, 0, 0,
      0, 0, a.z, 0,
      0, 0, 0, 1
  };
}

FUNCTION
Mat4 mat4_translation(Vec3 a) {
  return {
    1, 0, 0, a.x,
    0, 1, 0, a.y,
    0, 0, 1, a.z,
    0, 0, 0, 1
  };
}

FUNCTION
Mat4 mat4_rotation_z(float rotation) {
  float s = sinf(rotation);
  float c = cosf(rotation);
  Mat4 result = {
    c, s, 0, 0,
    -s,  c, 0, 0,
    0,  0, 1, 0,
    0,  0, 0, 1,
  };
  return result;
}

FUNCTION
Mat4 mat4_rotation_y(float rotation) {
  float s = sinf(rotation);
  float c = cosf(rotation);
  Mat4 result = {
    c, 0, -s, 0,
    0, 1,  0, 0,
    s, 0,  c, 0,
    0, 0,  0, 1,
  };
  return result;
}

FUNCTION
Mat4 mat4_rotation_x(float rotation) {
  float s = sinf(rotation);
  float c = cosf(rotation);
  Mat4 result = {
    1,  0, 0, 0,
    0,  c, s, 0,
    0,  -s, c, 0,
    0,  0, 0, 1,
  };
  return result;
}

FUNCTION
Mat4 mat4_perspective(float fov, float window_x, float window_y, float znear, float zfar) {
  float aspect_ratio = window_y / window_x;
  float f = (1.f / tanf((fov/2.f)*deg2rad));
  Mat4 result = {
    aspect_ratio*f, 0, 0, 0,
    0, f, 0, 0,
    0, 0, (zfar)-(zfar-znear),(-zfar*znear)-(zfar - znear),
    0,0,1,0
  };
  return result;
}

FN Mat4 mat4_look_at(Vec3 pos, Vec3 target, Vec3 up) {
  Vec3 z = normalize(target - pos);
  Vec3 x = normalize(cross(up, z));
  Vec3 y = cross(z, x);
  Mat4 result = {
    x.x,x.y,x.z,-dot(x,pos),
    y.x,y.y,y.z,-dot(y,pos),
    z.x,z.y,z.z,-dot(z,pos),
    0,0,0,       1,
  };
  return result;
}

FUNCTION
Mat4 mat4_transpose(Mat4 a) {
  Mat4 result = a;
  result.p[0][1] = result.p[1][0];
  result.p[0][2] = result.p[2][0];
  result.p[0][3] = result.p[3][0];
  result.p[2][1] = result.p[1][2];
  result.p[3][1] = result.p[1][3];
  result.p[3][2] = result.p[2][3];
  return result;
}

FUNCTION
Mat4 mat4_translate(Mat4 a, Vec3 translation) {
  a.p[0][0] += translation.x;
  a.p[0][1] += translation.y;
  a.p[0][2] += translation.z;
  return a;
}