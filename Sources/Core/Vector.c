//
//  Vector.m
//  Carnivores
//
//  Created by Dmitry Nechay on 18.08.09.
//  Copyright 2009 Action Forms. All rights reserved.
//

#include "Common.h"
#include "FastMath.h"
#include "Vector.h"


float Vector3D_DistanceToVectorSQR(Vector3D *v, Vector3D *nv, Vector3D *p)
{    
	Vector3D vc = Vector3D_Subtract(p, v);
	Vector3D n = Vector3D_CrossProduct(nv, &vc);
	return (n.x * n.x + n.y * n.y + n.z * n.z);
}

float Vector3D_DistanceToVector(Vector3D *v, Vector3D *nv, Vector3D *p)
{    
	Vector3D vc = Vector3D_Subtract(p, v);
	Vector3D n = Vector3D_CrossProduct(nv, &vc);
	return sqrtf(n.x * n.x + n.y * n.y + n.z * n.z);
}

Vector3D Vector3D_GetPointOnQuadCurve(Vector3D *v1, Vector3D *v2, Vector3D *v3, float k)
{
	Vector3D p1 = Vector3D_GetPointOnSegment(v1, v2, k);
	Vector3D p2 = Vector3D_GetPointOnSegment(v2, v3, k);

	return Vector3D_GetPointOnSegment(&p1, &p2, k);
}

Vector3D Vector3D_GetPointOnQubicCurve(Vector3D *v1, Vector3D *v2, Vector3D *v3, Vector3D *v4, float k)
{
	Vector3D p1 = Vector3D_GetPointOnSegment(v1, v2, k);
	Vector3D p2 = Vector3D_GetPointOnSegment(v2, v3, k);
	Vector3D p3 = Vector3D_GetPointOnSegment(v3, v4, k);

	return Vector3D_GetPointOnQuadCurve(&p1, &p2, &p3, k);
}

void Plane_Create(Plane *plane, const Vector3D *v1, const Vector3D *v2, const Vector3D *v3)
{
	Vector3D _v1, _v2;
	_v1 = Vector3D_Subtract(v2, v1);
	_v2 = Vector3D_Subtract(v3, v1);
	
	plane->normal = Vector3D_CrossProduct(&_v1, &_v2);
	Vector3D_Normalize(&plane->normal);
	
	plane->dist = Vector3D_DotProduct(v1, &plane->normal);
}

float Plane_Distance(const Plane *plane, const Vector3D *point)
{
	return Vector3D_DotProduct(point, &plane->normal) - plane->dist;
}

void Frustum_Calculate(Frustum *f, Vector3D v[5])
{
	/*
	+---------------------------------+
	|\ 2                           3 /|
	| \                             / |
	|  \                           /  |
	|   \                         /   |
	|    \        Frustum        /    |
	|     \                     /     |
	|      \                   /      |
	| 1     \                 /     4 |
	+ - - - -\- - - - - - - -/- - - - +
	  \       \             /       /
		\      \           /      /
		  \     \         /     /
			\	 \       /    /
	          \   \  0  /   /
			    \  \   /  /
				  \ \-/ /
	                \-/ 

	*/
	Plane_Create(&f->planes[frLeft], &v[0], &v[1], &v[2]);
	Plane_Create(&f->planes[frRight], &v[0], &v[3], &v[4]);
	Plane_Create(&f->planes[frBottom], &v[0], &v[4], &v[1]);
	Plane_Create(&f->planes[frTop], &v[0], &v[2], &v[3]);
	Plane_Create(&f->planes[frFar], &v[1], &v[4], &v[3]);
}

int Frustum_IsPointInside(const Frustum *f, const Vector3D *point)
{
	for (U32 i = 0; i < 5; i ++)
		if (Plane_Distance(&f->planes[i], point) <= -90.0f)
			return FALSE;
	
	return TRUE;
}

int Frustum_IsSphereInside(const Frustum *f, const Vector3D *sph_center, const float sph_sqr_radius)
{
	int result = InsideFrustum;
	float sqr_dist;
	
	for (U32 i = 0; i < 5; i ++)
	{
		sqr_dist = Vector3D_DotProduct(sph_center, &f->planes[i].normal) - f->planes[i].dist;
		
		sqr_dist = (sqr_dist < 0) ? -SQR(sqr_dist) : SQR(sqr_dist);
			
		if (sqr_dist <= -sph_sqr_radius)
			return OutsideFrustum;
		else if (sqr_dist <= sph_sqr_radius)
			result = ClipsFrustum;
	}
	
	return result;
}

int Trace_CheckPlane(Vector3D *a, Vector3D *b, Vector3D *c, Vector3D *trace_a, Vector3D *trace_b, Vector3D *trace_nv)
{
	Vector3D trace_plane_nv;
	Vector3D trace_hp;
	Vector3D trace_v1, trace_v2, trace_v3, trace_v4;

	float trace_ca, trace_cb;
	float trace_ccln, trace_ccvn;

	trace_v3 = Vector3D_Subtract(b, a);
	trace_v2 = Vector3D_Subtract(c, a);
	
	trace_plane_nv = Vector3D_CrossProduct(&trace_v3, &trace_v2);
	
	Vector3D_Normalize(&trace_plane_nv);
	
	
	trace_v1 = Vector3D_Subtract(trace_a, a);
	trace_v2 = Vector3D_Subtract(trace_b, a);
	
	trace_ca = Vector3D_DotProduct(&trace_v1, &trace_plane_nv);
	trace_cb = Vector3D_DotProduct(&trace_v2, &trace_plane_nv);
	
	if (trace_ca * trace_cb > -1.0f)
		return 0;
	
	
	//=================== calc hit point ===================//
	
	trace_ccln = Vector3D_DotProduct(&trace_v1, &trace_plane_nv);
	trace_ccvn = Vector3D_DotProduct(trace_nv, &trace_plane_nv);
	
	if (trace_ccvn == 0.0f)
		return 0;
	
	trace_ccln /= trace_ccvn;
	trace_ccln = fabsf(trace_ccln);
	trace_hp.x = trace_a->x + trace_nv->x * trace_ccln;
	trace_hp.y = trace_a->y + trace_nv->y * trace_ccln;
	trace_hp.z = trace_a->z + trace_nv->z * trace_ccln;  
	
	
	trace_v2 = Vector3D_Subtract(&trace_hp, a);	
	trace_v4 = Vector3D_CrossProduct(&trace_v3, &trace_v2);
	trace_ca = Vector3D_DotProduct(&trace_v4, &trace_plane_nv);
	if (trace_ca < 0.0f)
		return 0;
	
	trace_v1 = Vector3D_Subtract(c, b);
	trace_v2 = Vector3D_Subtract(&trace_hp, b);
	trace_v4 = Vector3D_CrossProduct(&trace_v1, &trace_v2);
	trace_ca = Vector3D_DotProduct(&trace_v4, &trace_plane_nv);
	if (trace_ca < 0.0f)
		return 0;
	
	trace_v1 = Vector3D_Subtract(a, c);
	trace_v2 = Vector3D_Subtract(&trace_hp, c);
	trace_v4 = Vector3D_CrossProduct(&trace_v1, &trace_v2);
	trace_ca = Vector3D_DotProduct(&trace_v4, &trace_plane_nv);
	if (trace_ca < 0.0f)
		return 0;
	
	if (Vector3D_SegmentLengthSQR(&trace_hp, trace_a) < Vector3D_SegmentLengthSQR(trace_b, trace_a))
	{
		*trace_b = trace_hp;
		return 1;
	}
	
	return 0;
}

void Plane_FindClosestPointInTriangle(const Vector3D * PointA, const Vector3D * PointB, const Vector3D * PointC, const Vector3D * DistPoint, Vector3D * PointInTriangle) // TODO: not my style
{
	Vector3D TriangleVectorA = Vector3D_Subtract(PointB, PointA);
	float TriangleVectorADistanceSQR = TriangleVectorA.x * TriangleVectorA.x + TriangleVectorA.y * TriangleVectorA.y + TriangleVectorA.z * TriangleVectorA.z;

	Vector3D TriangleVectorB = Vector3D_Subtract(PointC, PointA);
	float TriangleVectorBDistanceSQR = TriangleVectorB.x * TriangleVectorB.x + TriangleVectorB.y * TriangleVectorB.y + TriangleVectorB.z * TriangleVectorB.z;

	Vector3D TriangleVectorC = Vector3D_Subtract(DistPoint, PointA);
	
	float QA = Vector3D_DotProduct(&TriangleVectorA, &TriangleVectorC) / TriangleVectorADistanceSQR;

	float QB = Vector3D_DotProduct(&TriangleVectorB, &TriangleVectorC) / TriangleVectorBDistanceSQR;
	
	if ((QA >= 0.0f) && (QB >= 0.0f) && ((QA + QB) <= 1.0f))
	{
		// projected of DistPoint in triangle
		PointInTriangle->x = PointA->x + TriangleVectorA.x * QA + TriangleVectorB.x * QB;
		PointInTriangle->y = PointA->y + TriangleVectorA.y * QA + TriangleVectorB.y * QB;
		PointInTriangle->z = PointA->z + TriangleVectorA.z * QA + TriangleVectorB.z * QB;
	}
	else
	{
		if (QA <= 0.0f)
		{
			if (QB <= 0.0f)
			{
				PointInTriangle->x = PointA->x;
				PointInTriangle->y = PointA->y;
				PointInTriangle->z = PointA->z;
			}
			else if (QB >= 1.0f)
			{
				PointInTriangle->x = PointC->x;
				PointInTriangle->y = PointC->y;
				PointInTriangle->z = PointC->z;
			}
			else
			{
				PointInTriangle->x = PointA->x + TriangleVectorB.x * QB;
				PointInTriangle->y = PointA->y + TriangleVectorB.y * QB;
				PointInTriangle->z = PointA->z + TriangleVectorB.z * QB;
			}
		}
		else if (QB <= 0.0f)
		{
			if (QA >= 1.0f)
			{
				PointInTriangle->x = PointB->x;
				PointInTriangle->y = PointB->y;
				PointInTriangle->z = PointB->z;
			}
			else
			{
				PointInTriangle->x = PointA->x + TriangleVectorA.x * QA;
				PointInTriangle->y = PointA->y + TriangleVectorA.y * QA;
				PointInTriangle->z = PointA->z + TriangleVectorA.z * QA;
			}
		}
		else
		{
			if (QA >= QB + 1)
			{
				PointInTriangle->x = PointB->x;
				PointInTriangle->y = PointB->y;
				PointInTriangle->z = PointB->z;
			}
			else if (QA <= QB - 1)
			{
				PointInTriangle->x = PointC->x;
				PointInTriangle->y = PointC->y;
				PointInTriangle->z = PointC->z;
			}
			else
			{
				QA = (1.0f + QA - QB) / 2.0f;
				QB = 1.0f - QA;
				
				PointInTriangle->x = PointA->x + TriangleVectorA.x * QA + TriangleVectorB.x * QB;
				PointInTriangle->y = PointA->y + TriangleVectorA.y * QA + TriangleVectorB.y * QB;
				PointInTriangle->z = PointA->z + TriangleVectorA.z * QA + TriangleVectorB.z * QB;
			}
		}
	}
}

float Vector2D_DistanceToSegmentSQR(const Vector2D *v1, const Vector2D *v2, const Vector2D *p)
{
	Vector2D vd1, vd2, projection;

	const float lsqr = Vector2D_SegmentLengthSQR(v1, v2);

	if (lsqr == 0.0f)
		return Vector2D_SegmentLengthSQR(v1, p);

	vd1 = Vector2D_Subtract(p, v1);
	vd2 = Vector2D_Subtract(v2, v1);

	float plsqr = Vector2D_DotProduct(&vd1, &vd2) / lsqr;

	if (plsqr < 0.0f)
		return Vector2D_SegmentLengthSQR(v1, p);
	else if (plsqr > 1.0f)
		return Vector2D_SegmentLengthSQR(v2, p);

	projection.x = v1->x + plsqr * (v2->x - v1->x);
	projection.y = v1->y + plsqr * (v2->y - v1->y);

	return Vector2D_SegmentLengthSQR(p, &projection);
}

float Vector2D_DistanceToSegment(const Vector2D *v1, const Vector2D *v2, const Vector2D *p)
{
	return sqrtf(Vector2D_DistanceToSegmentSQR(v1, v2, p));
}
