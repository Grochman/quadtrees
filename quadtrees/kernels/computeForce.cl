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
    Particle particle;
    int particleId;
} QuadTreeNode;


inline void updateAcceleration(Particle* particle, float2 force) {
    particle->acceleration.x += force.x / particle->mass;
    particle->acceleration.y += force.y / particle->mass;
};

inline float2 calculateForce(float2 distance, float mass, Particle* particle) {
    float2 unit_vector = normalize(distance); 
    float distance_length = length(distance);
    distance_length = max(distance_length, MIN_DIST);
    float2 force = (mass * particle->mass / pow(distance_length, 2.f)) * unit_vector;
    return force;
}

inline float2 getTotalForce(Particle* particle, Particle* particles) {
    float2 totalForce = (float2)(.0f,.0f);
    int stack[MAX_STACK];
    int top = -1;
    stack[++top] = 0;
    while (top > -1) {
        int i = stack[top--];
        if (nodes[i]->particle == particle) {
            continue;
        }
        if (nodes[i]->particle) {
            float2 r = distance(particle->position, nodes[i]->particle->position);
            totalForce = add(totalForce, calculateForce(r, nodes[i]->particle->mass, particle));
            continue;
        }
        float s = nodes[i].dimensions.x;
        float d = length(distance(particle->position, nodes[i]->massCenter));
        if (s / d < 0.5f) {
            float2 r = distance(particle->position, nodes[i]->massCenter);
            totalForce = add(totalForce, calculateForce(r, nodes[i].mass, particle));
            continue;
        }
        else if (nodes[i]->leavesIdx != -1) {
            stack[++top] = nodes[i]->leavesIdx;
            stack[++top] = nodes[i]->leavesIdx + 1;
            stack[++top] = nodes[i]->leavesIdx + 2;
            stack[++top] = nodes[i]->leavesIdx + 3;
        }
    }
    
    return totalForce;
}


__kernel void computeForces(__global Particle * particles) {
    int id = get_global_id(0);
    Particle particle = particles[id];
    float2 force = getTotalForce(particle, particles);
    updateAcceleration(particle, force);
}
