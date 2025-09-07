typedef struct {
    float2 position;
    float2 velocity;
    float2 acceleration;
    float mass;
} Particle;

__kernel void updatePositions(__global Particle * particles, float dt) {
    int id = get_global_id(0);
    particles[id].velocity.x += particles[id].acceleration.x * dt * 0.5f;
    particles[id].velocity.y += particles[id].acceleration.y * dt * 0.5f;
    particles[id].position.x += particles[id].velocity.x * dt;
    particles[id].position.y += particles[id].velocity.y * dt;
    particles[id].velocity.x += particles[id].acceleration.x * dt * 0.5f;
    particles[id].velocity.y += particles[id].acceleration.y * dt * 0.5f;
    particles[id].acceleration.x = 0.0f;
    particles[id].acceleration.y = 0.0f;
}
