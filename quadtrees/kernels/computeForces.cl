#define MAX_STACK 64
#define MIN_DIST 0.1f

typedef struct {
    float2 position;
    float2 velocity;
    float2 acceleration;
    float mass;
} Particle;

typedef struct {
    float2 position;
    float2 dimensions;
    float2 massCenter;
    float mass;
    int leavesIdx;
    int particleIdx;
} QuadTreeNode;


inline void updateAcceleration(__global Particle* particle, float2 force) {
    particle->acceleration.x += force.x / particle->mass;
    particle->acceleration.y += force.y / particle->mass;
};

inline float2 calculateForce(float2 distance, float mass, __global Particle* particle) {
    float2 unit_vector = normalize(distance); 
    float distance_length = length(distance);
    distance_length = max(distance_length, MIN_DIST);
    float2 force = (mass * particle->mass / pow(distance_length, 2.f)) * unit_vector;
    return force;
}

inline float2 getTotalForce(int particleIdx, __global Particle* particles, __global QuadTreeNode* nodes) {
    float2 totalForce = (float2)(.0f,.0f);
    int stack[MAX_STACK];
    int top = -1;
    stack[++top] = 0;
    while (top > -1) {
        int i = stack[top--];
        
        if (nodes[i].particleIdx == particleIdx) {
            continue;
        }
        if (nodes[i].particleIdx != -1) {
            float2 r = particles[nodes[i].particleIdx].position - particles[particleIdx].position;
            totalForce = totalForce + calculateForce(r, particles[nodes[i].particleIdx].mass, &particles[particleIdx]);
            continue;
        }
        const float s = nodes[i].dimensions.x;
        const float d = length(nodes[i].massCenter - particles[particleIdx].position);
        if (s / d < 0.5) {
            float2 r = nodes[i].massCenter - particles[particleIdx].position;
            totalForce = totalForce + calculateForce(r, nodes[i].mass, &particles[particleIdx]);
            continue;
        }
        else if (nodes[i].leavesIdx != -1) {
            stack[++top] = nodes[i].leavesIdx;
            stack[++top] = nodes[i].leavesIdx + 1;
            stack[++top] = nodes[i].leavesIdx + 2;
            stack[++top] = nodes[i].leavesIdx + 3;
        }
    }
    
    return totalForce;
}


__kernel void computeForces(__global Particle * particles, __global QuadTreeNode * nodes) {
    int id = get_global_id(0);
    float2 force = getTotalForce(id, particles, nodes);
    updateAcceleration(&particles[id], force);
}
