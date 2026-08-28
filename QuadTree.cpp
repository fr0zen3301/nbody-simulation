#include "QuadTree.hpp"
#include <cmath>

QuadTree::QuadTree(const Vector2D& center, double halfSize) {
    root = std::make_unique<Node>();
    root->center = center;
    root->halfSize = halfSize;
}

void QuadTree::insert(const Body* b) {
    insert(*root, b, 0);
}

Vector2D QuadTree::computeForce(const Body& body, double theta) const {
    return force(*root, body, theta);
}

int QuadTree::quadrantOf(const Node& node, const Vector2D& p) {
    int q = 0;
    if (p.x >= node.center.x) q += 1;
    if (p.y >= node.center.y) q += 2;
    return q;
}

void QuadTree::subdivide(Node& node) {
    double h = node.halfSize / 2.0;
    const Vector2D offsets[4] = {
        {-h, -h}, {+h, -h}, {-h, +h}, {+h, +h}
    };
    for (int q = 0; q < 4; ++q) {
        node.children[q] = std::make_unique<Node>();
        node.children[q]->center = node.center + offsets[q];
        node.children[q]->halfSize = h;
    }
}

void QuadTree::insert(Node& node, const Body* b, int depth) {
    // Depth cap: bodies at (nearly) identical positions would subdivide forever
    constexpr int MAX_DEPTH = 32;
    if (node.mass == 0.0 && node.isLeaf()) {
        // empty leaf: take the body
        node.body = b;
        node.mass = b->mass;
        node.com = b->position;
        return;
    }

    if (node.isLeaf() && depth < MAX_DEPTH) {
        // occupied leaf
        const Body* old = node.body;
        node.body = nullptr;
        subdivide(node);
        insert(*node.children[quadrantOf(node, old->position)], old, depth + 1);
    }

    // update this node's aggregate mass
    double total = node.mass + b->mass;
    node.com = (node.com * node.mass + b->position * b->mass) / total;
    node.mass = total;

    if (!node.isLeaf()) {
        insert(*node.children[quadrantOf(node, b->position)], b, depth + 1);
    }
    // at MAX_DEPTH: aggregate updated above, body not stored individually
}

Vector2D QuadTree::force(const Node& node, const Body& b, double theta) {
    if (node.mass == 0.0) return Vector2D(0, 0);
    if (node.body == &b) return Vector2D(0, 0);

    Vector2D direction = node.com - b.position;
    double dist = direction.magnitude();

    bool farEnough = (2.0 * node.halfSize) / dist < theta;

    if (node.isLeaf() || farEnough) {
        if (dist < 1e-9) return Vector2D(0, 0);
        double distSq = dist * dist + Body::SOFTENING * Body::SOFTENING;
        double f = Body::G * b.mass * node.mass / distSq;
        return (direction / dist) * f;
    }

    Vector2D total(0, 0);
    for (const auto& child : node.children)
        total += force(*child, b, theta);
    return total;
}