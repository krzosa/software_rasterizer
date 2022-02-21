
FUNCTION
Mat4 make_matrix_rotation_x(float rotation) {
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
Mat4 make_matrix_rotation_y(float rotation) {
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
Mat4 make_matrix_rotation_z(float rotation) {
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
Mat4 make_matrix_perspective(float fov, float window_x, float window_y, float znear, float zfar) {
  float aspect_ratio = window_y / window_x;
  float f = (1.f / tanf((fov/2.f)*(180.f/PI32)));
  Mat4 result = {
    aspect_ratio*f, 0, 0, 0,
    0, f, 0, 0,
    0, 0, (zfar)-(zfar-znear),(-zfar*znear)-(zfar - znear),
    0,0,1,0
  };
  return result;
}

FUNCTION
Mat4 transpose(Mat4 a) {
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
Mat4 translate(Mat4 a, Vec3 translation) {
  a.p[0][0] += translation.x;
  a.p[0][1] += translation.y;
  a.p[0][2] += translation.z;
  return a;
}