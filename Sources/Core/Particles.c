
#include "Common.h"

#include "FastMath.h"
#include "Vector.h"
#include "Render.h"
#include "TextureManager.h"
#include "Particles.h"


#define MAX_PARTICLES_COUNT 16

typedef struct _Particle
{
	int texture_id;

} Particle;

Particle particles[MAX_PARTICLES_COUNT];
int particles_count;

float particle_vertices[12];
I16 particle_tex_coords[8];

void Particles_Init()
{
	particle_tex_coords[0] = 1;
    particle_tex_coords[1] = 1;
    particle_tex_coords[2] = 1;
    particle_tex_coords[3] = 0;
    particle_tex_coords[4] = 0;
    particle_tex_coords[5] = 1;
    particle_tex_coords[6] = 0;
    particle_tex_coords[7] = 0;
    
    particle_vertices[0] = 0.5f;
    particle_vertices[1] = -0.5f;
    particle_vertices[2] = 0.0f;
    
    particle_vertices[3] = 0.5f;
    particle_vertices[4] = 0.5f;
    particle_vertices[5] = 0.0f;
    
    particle_vertices[6] = -0.5f;
    particle_vertices[7] = -0.5f;
    particle_vertices[8] = 0.0f;
    
    particle_vertices[9] = -0.5f;
    particle_vertices[10] = 0.5f;
    particle_vertices[11] = 0.0f;

	particles_count = 0;
}

void Particles_AddParticle(ParticleId *particle_id, char *texture)
{
	if (particles_count >= MAX_PARTICLES_COUNT)
		return;

	particles[particles_count].texture_id = TexManager_AddTexture(texture, FLAG_PARTICLE_TEXTURE);

	*particle_id = particles_count;

	particles_count ++;
}

void Particles_RenderParticle(ParticleId particle_id, Vector3D *position, float rotation, float radius, float *color, Camera *camera)
{
	if ((particle_id >= MAX_PARTICLES_COUNT) || (particle_id < 0))
		return;

	float mm[4][4];
	float ca, sa, cb, sb, cg, sg;
	
    FastSinCos(DEG2RAD(-camera->alpha), &sa, &ca);
	FastSinCos(DEG2RAD(camera->beta), &sb, &cb);
	FastSinCos(rotation, &sg, &cg);

	mm[0][0] = (cg * ca + sg * sb * sa) * radius;  mm[1][0] = (cg * sb * sa - sg * ca) * radius;  mm[2][0] = (cb * sa) * radius;  mm[3][0] = position->x;
	mm[0][1] = (sg * cb) * radius;                 mm[1][1] = (cg * cb) * radius;                 mm[2][1] = (-sb) * radius;      mm[3][1] = position->y;
	mm[0][2] = (sg * sb * ca - cg * sa) * radius;  mm[1][2] = (sg * sa + cg * sb * ca) * radius;  mm[2][2] = (cb * ca) * radius;  mm[3][2] = position->z;
	mm[0][3] = 0.0f;                               mm[1][3] = 0.0f;                               mm[2][3] = 0.0f;                mm[3][3] = 1.0f;
	
	Render_SetMatrixMode(TR_MODEL);
	Render_SetMatrix(&mm[0][0]);

	Render_EnableTextures();
	Render_EnableVertexArray();
    Render_DisableColorArray();
    Render_EnableTexCoordArray();
    Render_DisableIndexArray();

	TexManager_SetTextureByIndex(particles[particle_id].texture_id);

	Render_SetVertexArray(particle_vertices, 3, TR_FLOAT, 0);
    Render_SetColor(color[0], color[1], color[2], color[3]);
    Render_SetTexCoordArray(particle_tex_coords, 2, TR_SHORT, 0);
	
    Render_DrawArrays(TR_TRIANGLE_STRIP, 4);
}

void Particles_Release()
{
	TexManager_RemoveTexturesByFlag(FLAG_PARTICLE_TEXTURE);
}

