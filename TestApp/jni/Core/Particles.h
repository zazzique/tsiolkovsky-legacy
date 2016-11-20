
#ifndef _PARTICLES_H
#define _PARTICLES_H

typedef int ParticleId;

void Particles_Init();
void Particles_AddParticle(ParticleId *particle_id, char *texture);
void Particles_RenderParticle(ParticleId particle_id, Vector3D *position, float rotation, float radius, float *color, Camera *camera);
void Particles_Release();

#endif /*_PARTICLES_H*/



