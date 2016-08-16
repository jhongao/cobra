#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

struct Color { unsigned char b, g, r, a; };
struct Texture { int width, height; std::vector<Color> data; };
struct Index { int pos[3], uv[3], normal[3]; };
struct Vector4 {
	float x, y, z, w;
	Vector4 operator- (const Vector4 &rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z }; }
	Vector4 Cross (const Vector4 &rhs) const { return { y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x }; }
	Vector4 Normalize () const { float invlen = 1.0f / sqrtf (x * x + y * y + z * z); return { x * invlen, y * invlen, z * invlen }; };
};
struct Vertex { Vector4 pos, uv, normal; };
struct Matrix4 {
	float m[4][4];
	Matrix4 () { memset (m, 0, sizeof (m)); m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f; }
    Matrix4 operator* (const Matrix4 &rhs) const {
        Matrix4 res;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                res.m[i][j] = m[i][0] * rhs.m[0][j] + m[i][1] * rhs.m[1][j] + m[i][2] * rhs.m[2][j] + m[i][3] * rhs.m[3][j];
            }
        }
        return res;
    }
	void Invert () {
		Matrix4	temm = *this;
		float tmp[12];
		tmp[0] = temm.m[2][2] * temm.m[3][3];
		tmp[1] = temm.m[3][2] * temm.m[2][3];
		tmp[2] = temm.m[1][2] * temm.m[3][3];
		tmp[3] = temm.m[3][2] * temm.m[1][3];
		tmp[4] = temm.m[1][2] * temm.m[2][3];
		tmp[5] = temm.m[2][2] * temm.m[1][3];
		tmp[6] = temm.m[0][2] * temm.m[3][3];
		tmp[7] = temm.m[3][2] * temm.m[0][3];
		tmp[8] = temm.m[0][2] * temm.m[2][3];
		tmp[9] = temm.m[2][2] * temm.m[0][3];
		tmp[10] = temm.m[0][2] * temm.m[1][3];
		tmp[11] = temm.m[1][2] * temm.m[0][3];
		m[0][0] = tmp[0] * temm.m[1][1] + tmp[3] * temm.m[2][1] + tmp[4] * temm.m[3][1];
		m[0][0] -= tmp[1] * temm.m[1][1] + tmp[2] * temm.m[2][1] + tmp[5] * temm.m[3][1];
		m[0][1] = tmp[1] * temm.m[0][1] + tmp[6] * temm.m[2][1] + tmp[9] * temm.m[3][1];
		m[0][1] -= tmp[0] * temm.m[0][1] + tmp[7] * temm.m[2][1] + tmp[8] * temm.m[3][1];
		m[0][2] = tmp[2] * temm.m[0][1] + tmp[7] * temm.m[1][1] + tmp[10] * temm.m[3][1];
		m[0][2] -= tmp[3] * temm.m[0][1] + tmp[6] * temm.m[1][1] + tmp[11] * temm.m[3][1];
		m[0][3] = tmp[5] * temm.m[0][1] + tmp[8] * temm.m[1][1] + tmp[11] * temm.m[2][1];
		m[0][3] -= tmp[4] * temm.m[0][1] + tmp[9] * temm.m[1][1] + tmp[10] * temm.m[2][1];
		m[1][0] = tmp[1] * temm.m[1][0] + tmp[2] * temm.m[2][0] + tmp[5] * temm.m[3][0];
		m[1][0] -= tmp[0] * temm.m[1][0] + tmp[3] * temm.m[2][0] + tmp[4] * temm.m[3][0];
		m[1][1] = tmp[0] * temm.m[0][0] + tmp[7] * temm.m[2][0] + tmp[8] * temm.m[3][0];
		m[1][1] -= tmp[1] * temm.m[0][0] + tmp[6] * temm.m[2][0] + tmp[9] * temm.m[3][0];
		m[1][2] = tmp[3] * temm.m[0][0] + tmp[6] * temm.m[1][0] + tmp[11] * temm.m[3][0];
		m[1][2] -= tmp[2] * temm.m[0][0] + tmp[7] * temm.m[1][0] + tmp[10] * temm.m[3][0];
		m[1][3] = tmp[4] * temm.m[0][0] + tmp[9] * temm.m[1][0] + tmp[10] * temm.m[2][0];
		m[1][3] -= tmp[5] * temm.m[0][0] + tmp[8] * temm.m[1][0] + tmp[11] * temm.m[2][0];
		tmp[0] = temm.m[2][0]*temm.m[3][1];
		tmp[1] = temm.m[3][0]*temm.m[2][1];
		tmp[2] = temm.m[1][0]*temm.m[3][1];
		tmp[3] = temm.m[3][0]*temm.m[1][1];
		tmp[4] = temm.m[1][0]*temm.m[2][1];
		tmp[5] = temm.m[2][0]*temm.m[1][1];
		tmp[6] = temm.m[0][0]*temm.m[3][1];
		tmp[7] = temm.m[3][0]*temm.m[0][1];
		tmp[8] = temm.m[0][0]*temm.m[2][1];
		tmp[9] = temm.m[2][0]*temm.m[0][1];
		tmp[10] = temm.m[0][0]*temm.m[1][1];
		tmp[11] = temm.m[1][0]*temm.m[0][1];
		m[2][0] = tmp[0] * temm.m[1][3] + tmp[3] * temm.m[2][3] + tmp[4] * temm.m[3][3];
		m[2][0] -= tmp[1] * temm.m[1][3] + tmp[2] * temm.m[2][3] + tmp[5] * temm.m[3][3];
		m[2][1] = tmp[1] * temm.m[0][3] + tmp[6] * temm.m[2][3] + tmp[9] * temm.m[3][3];
		m[2][1] -= tmp[0] * temm.m[0][3] + tmp[7] * temm.m[2][3] + tmp[8] * temm.m[3][3];
		m[2][2] = tmp[2] * temm.m[0][3] + tmp[7] * temm.m[1][3] + tmp[10] * temm.m[3][3];
		m[2][2] -= tmp[3] * temm.m[0][3] + tmp[6] * temm.m[1][3] + tmp[11] * temm.m[3][3];
		m[2][3] = tmp[5] * temm.m[0][3] + tmp[8] * temm.m[1][3] + tmp[11] * temm.m[2][3];
		m[2][3] -= tmp[4] * temm.m[0][3] + tmp[9] * temm.m[1][3] + tmp[10] * temm.m[2][3];
		m[3][0] = tmp[2] * temm.m[2][2] + tmp[5] * temm.m[3][2] + tmp[1] * temm.m[1][2];
		m[3][0] -= tmp[4] * temm.m[3][2] + tmp[0] * temm.m[1][2] + tmp[3] * temm.m[2][2];
		m[3][1] = tmp[8] * temm.m[3][2] + tmp[0] * temm.m[0][2] + tmp[7] * temm.m[2][2];
		m[3][1] -= tmp[6] * temm.m[2][2] + tmp[9] * temm.m[3][2] + tmp[1] * temm.m[0][2];
		m[3][2] = tmp[6] * temm.m[1][2] + tmp[11] * temm.m[3][2] + tmp[3] * temm.m[0][2];
		m[3][2] -= tmp[10] * temm.m[3][2] + tmp[2] * temm.m[0][2] + tmp[7] * temm.m[1][2];
		m[3][3] = tmp[10] * temm.m[2][2] + tmp[4] * temm.m[0][2] + tmp[9] * temm.m[1][2];
		m[3][3] -= tmp[8] * temm.m[1][2] + tmp[11] * temm.m[2][2] + tmp[5] * temm.m[0][2];
		float idet = 1.0f / (temm.m[0][0] * m[0][0] + temm.m[1][0] * m[0][1] + temm.m[2][0] * m[0][2] + temm.m[3][0] * m[0][3]);
		m[0][0] *= idet; m[0][1] *= idet; m[0][2] *= idet; m[0][3] *= idet;
		m[1][0] *= idet; m[1][1] *= idet; m[1][2] *= idet; m[1][3] *= idet;
		m[2][0] *= idet; m[2][1] *= idet; m[2][2] *= idet; m[2][3] *= idet;
		m[3][0] *= idet; m[3][1] *= idet; m[3][2] *= idet; m[3][3] *= idet;
	}
	void Translate (const Vector4 &v) { m[3][0] = v.x; m[3][1] = v.y; m[3][2] = v.z; }
};
Vector4 operator* (const Vector4 &b, const Matrix4 &mat) {
	Vector4 v;
	v.w = mat.m[0][3] * b.x + mat.m[1][3] * b.y + mat.m[2][3] * b.z + mat.m[3][3];
	v.x = (mat.m[0][0] * b.x + mat.m[1][0] * b.y + mat.m[2][0] * b.z + mat.m[3][0]) / v.w;
	v.y = (mat.m[0][1] * b.x + mat.m[1][1] * b.y + mat.m[2][1] * b.z + mat.m[3][1]) / v.w;
	v.z = (mat.m[0][2] * b.x + mat.m[1][2] * b.y + mat.m[2][2] * b.z + mat.m[3][2]) / v.w;
	return v;
}
Matrix4 CreateProjectionMatrix (float fov, float ratio, float n, float f) {
	float r = n * tan (fov * 0.5f), l = -r, b = -r / ratio, t = r / ratio;
	Matrix4 mat;
	mat.m[0][0] = 2 * n / (r - l); mat.m[0][1] = 0.0f; mat.m[0][2] = 0.0f; mat.m[0][3] = 0.0f;
	mat.m[1][0] = 0.0f; mat.m[1][1] = 2 * n / (t - b); mat.m[1][2] = 0.0f; mat.m[1][3] = 0.0f;
	mat.m[2][0] = (r + l) / (r - l); mat.m[2][1] = (t + b) / (t - b); mat.m[2][2] = -(f + n) / (f - n); mat.m[2][3] = -1.0f;
	mat.m[3][0] = 0.0f; mat.m[3][1] = 0.0f; mat.m[3][2] = (-2.0f * f * n) / (f - n); mat.m[3][3] = 0.0f;
	return mat;
}
Matrix4 CreateViewMatrix (const Vector4 &look, const Vector4 &at, const Vector4 &up) {
	Vector4 dir = (look - at).Normalize (), left = up.Cross (dir).Normalize (), newUp = dir.Cross (left);
	Matrix4 mat;
	mat.m[0][0] = left.x; mat.m[0][1] = left.y; mat.m[0][2] = left.z; mat.m[0][3] = 0.0f;
	mat.m[1][0] = newUp.x; mat.m[1][1] = newUp.y; mat.m[1][2] = newUp.z; mat.m[1][3] = 0.0f;
	mat.m[2][0] = dir.x; mat.m[2][1] = dir.y; mat.m[2][2] = dir.z; mat.m[2][3] = 0.0f;
	mat.m[3][0] = look.x; mat.m[3][1] = look.y; mat.m[3][2] = look.z; mat.m[3][3] = 1.0f;
	mat.Invert ();
	return mat;
}

void SaveBmp (std::vector<Color> &frameBuffer, int width, int height, std::string file) {
#define INT2CHAR_BIT(num, bit) (char)(((num) >> (bit)) & 0xff)
#define INT2CHAR(num) INT2CHAR_BIT((num),0), INT2CHAR_BIT((num),8), INT2CHAR_BIT((num),16), INT2CHAR_BIT((num),24)
	const char header[54] = { 'B', 'M', INT2CHAR (54 + width*height * sizeof (Color)), INT2CHAR (0), INT2CHAR (54), INT2CHAR (40), INT2CHAR (width), INT2CHAR (height), 1, 0, 32, 0 };
	std::ofstream ofs (file, std::ios_base::out | std::ios_base::binary);
	ofs.write (header, sizeof (header));
	ofs.write (static_cast<const char *> (static_cast<const void *> (frameBuffer.data ())), width * height * sizeof (Color));
}
void LoadBmp (Texture &texture, std::string file) {
	std::ifstream is (file, std::ios_base::binary);
	unsigned char buf[54];
	is.read ((char *)buf, sizeof (buf));
	texture.width = *(int *)&buf[18], texture.height = *(int *)&buf[22];
	texture.data.resize (texture.width * texture.height);
	if (buf[28] == 24) for (auto &color : texture.data) is.read ((char *)&color, 3);
	else if (buf[28] == 32) is.read ((char *)&texture.data[0], sizeof (Color) * texture.width * texture.height);
}

struct Model {
	std::vector<Vector4> posBuffer, normalBuffer, uvBuffer;
	std::vector<Index> indexBuffer;
	Texture diffTexture;
	Matrix4 mat;
	Model (std::string name, const Vector4 &translate) : posBuffer (1, { 0 }), normalBuffer (1, { 0 }), uvBuffer (1, { 0 }) {
		mat.Translate (translate);
		LoadObj (name + ".obj");
		LoadBmp (diffTexture, name + ".bmp");
	}
	void LoadObj (std::string str) {
		float x, y, z;
		char dummy;
		std::ifstream is (str);
		while (std::getline (is, str)) {
			if (str.length () < 2) continue;
			std::istringstream iss (str);
			std::string token;
			if (str[1] == 't' && str[0] == 'v') {
				iss >> token >> x >> y;
				uvBuffer.push_back ({ x, y });
			} else if (str[1] == 'n' && str[0] == 'v') {
				iss >> token >> x >> y >> z;
				normalBuffer.push_back ({ x, y, z });
			} else if (str[0] == 'v') {
				iss >> token >> x >> y >> z;
				posBuffer.push_back ({ x, y, z });
			} else if (str[0] == 'f') {
				Index index = { 0 };
				if (str.find ("//") != std::string::npos) {
					iss >> token >> index.pos[0] >> dummy >> dummy >> index.normal[0] >>
						index.pos[1] >> dummy >> dummy >> index.normal[1] >>
						index.pos[2] >> dummy >> dummy >> index.normal[2];
				} else {
					size_t count = 0, pos = str.find ('/');
					while (pos != std::string::npos) { count++; pos = str.find ('/', pos + 1); }
					if (count == 6) {
						iss >> token >> index.pos[0] >> dummy >> index.uv[0] >> dummy >> index.normal[0] >>
							index.pos[1] >> dummy >> index.uv[1] >> dummy >> index.normal[1] >>
							index.pos[2] >> dummy >> index.uv[2] >> dummy >> index.normal[2];
					} else if (count == 3) {
						iss >> token >> index.pos[0] >> dummy >> index.uv[0] >> index.pos[1] >> dummy >> index.uv[1] >> index.pos[2] >> dummy >> index.uv[2];
					}
				}
				indexBuffer.push_back (index);
			}
		}
		for (auto &index : indexBuffer) {
			for (int i = 0; i < 3; i++) {
				if (index.pos[i] < 0) index.pos[i] += (int)posBuffer.size ();
				if (index.uv[i] < 0) index.uv[i] += (int)uvBuffer.size ();
				if (index.normal[i] < 0) index.normal[i] += (int)normalBuffer.size ();
			}
		}
	}
};

struct Renderer {
	int width, height;
	std::vector<Color> frameBuffer;
	std::vector<float> depthBuffer;
	Matrix4 projMat;

	Renderer (int w, int h, const Matrix4 &pm) : width (w), height (h), frameBuffer (w * h, { 0, 0, 0, 0 }), depthBuffer (w * h, std::numeric_limits<float>::max ()), projMat (pm) { }

	void DrawModel (Matrix4 viewMat, Model &model, bool drawTex = true, bool drawWireFrame = false) {
		Matrix4 mvp = model.mat * viewMat * projMat;
		auto VertexShader = [&mvp] (const Vector4 &pos, const Vector4 &, const Vector4 &uv, Vertex &outVertex) {
			outVertex.pos = pos * mvp;
			outVertex.uv = uv;
		};
		for (auto &index : model.indexBuffer) {
			Vertex ov[3];
			for (int i = 0; i < 3; i++) {
				VertexShader (model.posBuffer[index.pos[i]], model.normalBuffer[index.normal[i]], model.uvBuffer[index.uv[i]], ov[i]);
                ov[i].pos.x = (ov[i].pos.x + 1)* 0.5f * width;
                ov[i].pos.y = (ov[i].pos.y + 1)* 0.5f * height;
				ov[i].pos.z = ov[i].pos.w;
			}
            if (drawTex) FillTriangle (model, ov[0], ov[1], ov[2]);
			if (drawWireFrame) DrawTriangle (ov[0], ov[1], ov[2], { 0, 255, 0, 0 });
		}
	}

	void FillTriangle (Model &model, const Vertex &v0, const Vertex &v1, const Vertex &v2) {
		auto PixelShader = [] (const Color &diffColor, Color &outColor) {
			outColor = diffColor;
		};

		float w = EdgeFunc (v0.pos, v1.pos, v2.pos);
        int x0 = std::max (0, (int)std::floor (std::min (v0.pos.x, std::min (v1.pos.x, v2.pos.x))));
        int y0 = std::max (0, (int)std::floor (std::min (v0.pos.y, std::min (v1.pos.y, v2.pos.y))));
        int x1 = std::min (width - 1, (int)std::floor (std::max (v0.pos.x, std::max (v1.pos.x, v2.pos.x))));
        int y1 = std::min (height - 1, (int)std::floor (std::max (v0.pos.y, std::max (v1.pos.y, v2.pos.y))));
        for (int y = y0; y <= y1; y++) {
            for (int x = x0; x <= x1; x++) {
                Vector4 v = { x + 0.5f, y + 0.5f, 0 };
                float w0 = EdgeFunc (v1.pos, v2.pos, v) / w, w1 = EdgeFunc (v2.pos, v0.pos, v) / w, w2 = EdgeFunc (v0.pos, v1.pos, v) / w;
                if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
                    float z = 1.0f / (w0 / v0.pos.z + w1 / v1.pos.z + w2 / v2.pos.z);
                    if (z >= depthBuffer[x + y * width]) continue;
					Color outColor;
                    float s = std::min (0.999999f, std::max (0.0f, (w0 * v0.uv.x / v0.pos.z + w1 * v1.uv.x / v1.pos.z + w2 * v2.uv.x / v2.pos.z) * z));
					float t = std::min (0.999999f, std::max (0.0f, (w0 * v0.uv.y / v0.pos.z + w1 * v1.uv.y / v1.pos.z + w2 * v2.uv.y / v2.pos.z) * z));
                    PixelShader (model.diffTexture.data[(int)std::floor (s * model.diffTexture.width) + (int)std::floor (t  * model.diffTexture.height) * model.diffTexture.width], outColor);
					frameBuffer[x + y * width] = outColor;
					depthBuffer[x + y * width] = z;
                }
            }
        }
	}
    static inline float EdgeFunc (const Vector4 &p0, const Vector4 &p1, const Vector4 &p2) { return ((p2.x - p0.x) * (p1.y - p0.y) - (p2.y - p0.y) * (p1.x - p0.x)); }

	void DrawTriangle (const Vertex &v0, const Vertex &v1, const Vertex &v2, const Color &color) {
		DrawLine (v0, v1, color); DrawLine (v1, v2, color); DrawLine (v0, v2, color);
	}
	void DrawLine (const Vertex &v0, const Vertex &v1, const Color &color) {
		int x0 = (int)round (v0.pos.x), y0 = (int)round (v0.pos.y), x1 = (int)round (v1.pos.x), y1 = (int)round (v1.pos.y), octant = CalcOctant (x0, y0, x1, y1);
		SwitchToOctantZeroFrom (octant, x0, y0);
		SwitchToOctantZeroFrom (octant, x1, y1);
		float dx = (float)(x1 - x0), dy = (float)(y1 - y0), y = (float)y0, delta = dy / dx;
		for (int x = x0 + 1; x <= x1; x++, y += delta) {
			int ix = x, iy = (int)round (y);
			SwitchFromOctantZeroTo (octant, ix, iy);
			if (ix >= 0 && ix < width && iy >= 0 && iy < height) {
				frameBuffer[ix + iy * width] = color;
				depthBuffer[ix + iy * width] = 0;
			}
		}
	}
	static int CalcOctant (int x0, int y0, int x1, int y1)
	{
		int x = x1 - x0, y = y1 - y0;
		if (x >= 0)
		{
			if (y >= 0)
			{
				if (x > y)
					return 0;
				else
					return 1;
			} else
			{
				if (x > -y)
					return 7;
				else
					return 6;
			}
		} else
		{
			if (y >= 0)
			{
				if (-x > y)
					return 3;
				else
					return 2;
			} else
			{
				if (x > y)
					return 5;
				else
					return 4;
			}
		}
	}
	static void SwitchToOctantZeroFrom (int octant, int &x, int &y)
	{
		switch (octant)
		{
		case 1:
			std::swap (x, y);
			break;
		case 2:
			x = -x;
			std::swap (x, y);
			break;
		case 3:
			x = -x;
			break;
		case 4:
			x = -x;
			y = -y;
			break;
		case 5:
			x = -x;
			y = -y;
			std::swap (x, y);
			break;
		case 6:
			y = -y;
			std::swap (x, y);
			break;
		case 7:
			y = -y;
			break;
		}
	}
	static void SwitchFromOctantZeroTo (int octant, int &x, int &y)
	{
		switch (octant)
		{
		case 1:
			std::swap (x, y);
			break;
		case 2:
			y = -y;
			std::swap (x, y);
			break;
		case 3:
			x = -x;
			break;
		case 4:
			x = -x;
			y = -y;
			break;
		case 5:
			x = -x;
			y = -y;
			std::swap (x, y);
			break;
		case 6:
			x = -x;
			std::swap (x, y);
			break;
		case 7:
			y = -y;
			break;
		}
	}
};

int main () {
	const int WIDTH = 1024, HEIGHT = 768;
	Renderer renderer (WIDTH, HEIGHT, CreateProjectionMatrix ((float)M_PI_2, (float)WIDTH / (float)HEIGHT, 0.1f, 1000.0f));
	Model model ("cube", { 0.0f, 0.0f, 0.0f });
	renderer.DrawModel (CreateViewMatrix ({ 0.0f, 1.2f, 1.4f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }), model, true, false);
	SaveBmp (renderer.frameBuffer, WIDTH, HEIGHT, "screenshot.bmp");
	return 0;
}
