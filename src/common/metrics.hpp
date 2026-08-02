#pragma once
#include "../common/raylib_cpp.hpp"
#include "./euler_angles.hpp"

using namespace raylib;

namespace openAITD {

	struct Vector3i {
		int x, y, z;
	};

  class Metrics {
	public:

	  static const Matrix Metrics::roomMatrix;
		static const Matrix Metrics::roomMatrixInv;
	  static const Matrix Metrics::modelMatrix;
		static const Matrix Metrics::modelMatrixInv;		
	  static const float scale;

    static EulerAngles fromRotate(const int alpha, const int beta, const int gamma) {
			// if ( alpha == 0 && gamma == 0 ) {
			// 	return { 0, (beta+512) * scale, 0 };
			// }
			EulerAngles result = { 
				alpha * scale,
				beta * scale,
				gamma * scale
			};
      return result;
    }

    static EulerAngles fromRotate(const Vector3i rot) {
			return fromRotate(rot.x, rot.y, rot.z);
    }

		static int normInt(const float vf) {
			int v = (int)(roundf(vf));
			v %= 1024;
			if (v < 0) v += 1024;
			return v;
		}

  	//Reverse format for scripts
		static Vector3i toRotate(const Vector3 v) {
			// if ( v.x == 0 && v.z == 0 ) {
			// 	return { 0, normInt(v.y / scale - 512), 0 };
			// }
			Vector3i result = { 
				normInt(v.x / scale), 
				normInt(v.y / scale), 
				normInt(v.z / scale)
			};
			return result;
		}
		
  };

	inline const Matrix Metrics::roomMatrix = MatrixRotateX(PI); //raylib::MatrixScale(1, -1, -1);
	inline const Matrix Metrics::roomMatrixInv = MatrixInvert(Metrics::roomMatrix);
	inline const Matrix Metrics::modelMatrix = MatrixMultiply(MatrixRotateX(PI), MatrixRotateY(PI)); 
	inline const Matrix Metrics::modelMatrixInv = MatrixInvert(Metrics::modelMatrix);

	inline const float Metrics::scale = PI / 512.0f;
}