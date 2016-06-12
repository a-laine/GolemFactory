#pragma once




/*
*/
template <typename T, precision P>
tmat4x4<T, P> lookAt
(
	tvec3<T, P> const & eye,
	tvec3<T, P> const & center,
	tvec3<T, P> const & up
)
{
	tvec3<T, P> const f(normalize(center - eye));
	tvec3<T, P> const s(normalize(cross(f, up)));
	tvec3<T, P> const u(cross(s, f));

	tmat4x4<T, P> Result(1);
	Result[0][0] = s.x;
	Result[1][0] = s.y;
	Result[2][0] = s.z;
	Result[0][1] = u.x;
	Result[1][1] = u.y;
	Result[2][1] = u.z;
	Result[0][2] =-f.x;
	Result[1][2] =-f.y;
	Result[2][2] =-f.z;
	Result[3][0] =-dot(s, eye);
	Result[3][1] =-dot(u, eye);
	Result[3][2] = dot(f, eye);
	return Result;
}



/*
   Matrice de projection (perspective) :
       FOV : field of view
	   ratio : ration d'aspect (ratio de l'écran, 16/9)
	   zNear/zFar : Plan de découpe proche/lointain


   (1/(ratio*tan(FOV/2)))         0                      0                0
              0             (1/tan(FOV/2))               0                0
			  0                   0         -(zFar+zNear)/(zFar-zNear)    -1
			  0                   0        -(2*zFar*zNear)/(zFar-zNear)   0
*/
template <typename T>
tmat4x4<T, defaultp> perspective
(
	T fovy,
	T aspect,
	T zNear,
	T zFar
)
{
	assert(abs(aspect - std::numeric_limits<T>::epsilon()) > static_cast<T>(0));

	T const tanHalfFovy = tan(fovy / static_cast<T>(2));

	tmat4x4<T, defaultp> Result(static_cast<T>(0));
	Result[0][0] = static_cast<T>(1) / (aspect * tanHalfFovy);
	Result[1][1] = static_cast<T>(1) / (tanHalfFovy);
	Result[2][2] = -(zFar + zNear) / (zFar - zNear);
	Result[2][3] = -static_cast<T>(1);
	Result[3][2] = -(static_cast<T>(2) * zFar * zNear) / (zFar - zNear);
	return Result;
}