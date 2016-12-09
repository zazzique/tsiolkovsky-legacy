//
//  Vector.h
//  Carnivores
//
//  Created by Dmitry Nechay on 18.08.09.
//  Copyright 2009 Action Forms. All rights reserved.
//

#ifndef _VECTOR_H_
#define _VECTOR_H_

#define OutsideFrustum	0x00
#define InsideFrustum	0x01
#define ClipsFrustum	0x02

#define TRES_NONE	-1 // TODO: tot here for sure
#define TRES_GROUND  1
#define TRES_WATER   2
#define TRES_MODEL   3
#define TRES_CHAR    4


typedef struct _Vector2D
{
	float x;
	float y;

} Vector2D;

inline Vector2D Vector2D_Add(const Vector2D *v1, const Vector2D *v2)
{
	Vector2D r;
	r.x = v1->x + v2->x;
	r.y = v1->y + v2->y;
	return r;
}

inline Vector2D Vector2D_Subtract(const Vector2D *v1, const Vector2D *v2)
{
	Vector2D r;
	r.x = v1->x - v2->x;
	r.y = v1->y - v2->y;
	return r;
}

inline float Vector2D_DotProduct(const Vector2D *v1, const Vector2D *v2)
{
	return (v1->x * v2->x + v1->y * v2->y);
}

inline float Vector2D_LengthSQR(const Vector2D *v)
{
	return v->x * v->x + v->y * v->y;
}

inline float Vector2D_Length(const Vector2D *v)
{
	return sqrtf(v->x * v->x + v->y * v->y);
}

inline float Vector2D_SegmentLengthSQR(const Vector2D *v1, const Vector2D *v2)
{
	Vector2D v;
	v.x = v2->x - v1->x;
	v.y = v2->y - v1->y;
	return v.x * v.x + v.y * v.y;
}

inline float Vector2D_SegmentLength(const Vector2D *v1, const Vector2D *v2)
{
	Vector2D v;
	v.x = v2->x - v1->x;
	v.y = v2->y - v1->y;
	return sqrtf(v.x * v.x + v.y * v.y);
}

inline void Vector2D_Normalize(Vector2D *v)
{
	float n = 1.0f / (sqrtf(v->x * v->x + v->y * v->y) + FLT_EPSILON);
	v->x = v->x * n;
	v->y = v->y * n;  
}

float     Vector2D_DistanceToSegmentSQR(const Vector2D *v1, const Vector2D *v2, const Vector2D *p);
float     Vector2D_DistanceToSegment(const Vector2D *v1, const Vector2D *v2, const Vector2D *p);

typedef struct _Vector3D
{
	float x;
	float y;
	float z;

} Vector3D;

inline Vector3D Vector3D_Add(const Vector3D *v1, const Vector3D *v2)
{
	Vector3D r;
	r.x = v1->x + v2->x;
	r.y = v1->y + v2->y;
	r.z = v1->z + v2->z;
	return r;
}

inline Vector3D Vector3D_Subtract(const Vector3D *v1, const Vector3D *v2)
{
	Vector3D r;
	r.x = v1->x - v2->x;
	r.y = v1->y - v2->y;
	r.z = v1->z - v2->z;
	return r;
}

inline float Vector3D_DotProduct(const Vector3D *v1, const Vector3D *v2)
{
	return (v1->x * v2->x + v1->y * v2->y + v1->z * v2->z);
}

inline Vector3D Vector3D_CrossProduct(const Vector3D *v1, const Vector3D *v2)
{
	Vector3D r;
	r.x = v1->y * v2->z - v2->y * v1->z;
	r.y = v2->x * v1->z - v1->x * v2->z;
	r.z = v1->x * v2->y - v2->x * v1->y;
	return r;
}

inline void Vector3D_Normalize(Vector3D *v)
{
	float n = 1.0f / (sqrtf(v->x * v->x + v->y * v->y + v->z * v->z) + FLT_EPSILON);
	v->x = v->x * n;
	v->y = v->y * n;  
	v->z = v->z * n;
}

inline float Vector3D_LengthSQR(Vector3D *v)
{
	return v->x * v->x + v->y * v->y + v->z * v->z;
}

inline float Vector3D_Length(Vector3D *v)
{
	return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
}

inline float Vector3D_SegmentLengthSQR(const Vector3D *v1, const Vector3D *v2)
{
	Vector3D v;
	v.x = v2->x - v1->x;
	v.y = v2->y - v1->y;
	v.z = v2->z - v1->z;
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

inline float Vector3D_SegmentLength(const Vector3D *v1, const Vector3D *v2)
{
	Vector3D v;
	v.x = v2->x - v1->x;
	v.y = v2->y - v1->y;
	v.z = v2->z - v1->z;
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline Vector3D Vector3D_GetPointOnSegment(Vector3D *v1, Vector3D *v2, float k)
{
	Vector3D result;
	result.x = v1->x + (v2->x - v1->x) * k;
	result.y = v1->y + (v2->y - v1->y) * k;
	result.z = v1->z + (v2->z - v1->z) * k;
	return result;
}

Vector3D  Vector3D_GetPointOnQuadCurve(Vector3D *v1, Vector3D *v2, Vector3D *v3, float k);
Vector3D  Vector3D_GetPointOnQubicCurve(Vector3D *v1, Vector3D *v2, Vector3D *v3, Vector3D *v4, float k);
float     Vector3D_DistanceToVectorSQR(Vector3D *v, Vector3D *nv, Vector3D *p);
float     Vector3D_DistanceToVector(Vector3D *v, Vector3D *nv, Vector3D *p);


typedef struct _Plane
{
	Vector3D normal;
	float dist;

} Plane;

void Plane_Create(Plane *plane, const Vector3D *v1, const Vector3D *v2, const Vector3D *v3);
float Plane_Distance(const Plane *plane, const Vector3D *point);


enum FrustumPlanes
{
	frLeft,
	frRight,
	frBottom,
	frTop,
	frFar
};


typedef struct _Frustum
{
	Plane planes[5];

} Frustum;

void Frustum_Calculate(Frustum *f, Vector3D v[5]);
int  Frustum_IsPointInside(const Frustum *f, const Vector3D *point);
int  Frustum_IsSphereInside(const Frustum *f, const Vector3D *sph_center, const float sph_sqr_radius);


int Trace_CheckPlane(Vector3D *a, Vector3D *b, Vector3D *c, Vector3D *trace_a, Vector3D *trace_b, Vector3D *trace_nv);
void Plane_FindClosestPointInTriangle(const Vector3D * PointA, const Vector3D * PointB, const Vector3D * PointC, const Vector3D * DistPoint, Vector3D * PointInTriangle);


#endif /* _VECTOR_H_ */
