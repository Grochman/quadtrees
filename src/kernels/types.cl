typedef struct {
    float2 position;
    float2 velocity;
    float2 acceleration;
    float mass;
    float pad;
} Particle;

typedef struct {
    float2 position;
    float2 dimensions;
    float2 massCenter;
    float mass;
    int leavesIdx;
    int particleIdx;
    int pad;
} QuadTreeNode;
