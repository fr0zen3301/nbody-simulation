#pragma once
#include "Vector2D.hpp"
#include "Body.hpp"
#include <array>
#include <memory>

class QuadTree {
public:
    QuadTree(const Vector2D& center, double halfSize);

    void insert(const Body* b);

    // net gravitational force
    Vector2D computeForce(const Body& body, double theta) const;

private:
    struct Node {
        Vector2D center;        // geometric center of this square
        double halfSize;
        Vector2D com{0, 0};     // center of mass of all bodies inside
        double mass = 0.0;
        const Body* body = nullptr;
        std::array<std::unique_ptr<Node>, 4> children;      // NW, NE, SW, SE

        bool isLeaf() const { return !children[0]; }
    };

    std::unique_ptr<Node> root;

    static void insert(Node& node, const Body* b, int depth);
    static void subdivide(Node& node);
    static int quadrantOf(const Node& node, const Vector2D& p);
    static Vector2D force(const Node& node, const Body& b, double theta);
};