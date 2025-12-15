#pragma once

// このファイルは TrussC.h からインクルードされる

#include <vector>

namespace trussc {

// Polyline - 頂点配列を持つクラス
class Polyline {
public:
    Polyline() : closed_(false) {}

    // 頂点を追加
    void addVertex(float x, float y) {
        vertices_.push_back(Vec3{x, y, 0.0f});
    }

    void addVertex(float x, float y, float z) {
        vertices_.push_back(Vec3{x, y, z});
    }

    void addVertex(const Vec2& v) {
        addVertex(v.x, v.y);
    }

    void addVertex(const Vec3& v) {
        vertices_.push_back(v);
    }

    // 複数頂点を一度に追加
    void addVertices(const std::vector<Vec2>& verts) {
        for (const auto& v : verts) {
            addVertex(v);
        }
    }

    void addVertices(const std::vector<Vec3>& verts) {
        for (const auto& v : verts) {
            addVertex(v);
        }
    }

    // 頂点を取得
    const std::vector<Vec3>& getVertices() const {
        return vertices_;
    }

    std::vector<Vec3>& getVertices() {
        return vertices_;
    }

    // 頂点数
    size_t size() const {
        return vertices_.size();
    }

    bool empty() const {
        return vertices_.empty();
    }

    // 特定の頂点にアクセス
    Vec3& operator[](size_t index) {
        return vertices_[index];
    }

    const Vec3& operator[](size_t index) const {
        return vertices_[index];
    }

    // クリア
    void clear() {
        vertices_.clear();
        closed_ = false;
    }

    // 閉じる/開く
    void close() {
        closed_ = true;
    }

    void setClosed(bool closed) {
        closed_ = closed;
    }

    bool isClosed() const {
        return closed_;
    }

    // 描画
    void draw() const {
        if (vertices_.empty()) return;

        size_t n = vertices_.size();

        // fill モード: 三角形ファン（凸形状のみ正しく描画）
        if (internal::fillEnabled && n >= 3) {
            sgl_begin_triangles();
            sgl_c4f(internal::currentR, internal::currentG, internal::currentB, internal::currentA);
            for (size_t i = 1; i < n - 1; i++) {
                sgl_v3f(vertices_[0].x, vertices_[0].y, vertices_[0].z);
                sgl_v3f(vertices_[i].x, vertices_[i].y, vertices_[i].z);
                sgl_v3f(vertices_[i+1].x, vertices_[i+1].y, vertices_[i+1].z);
            }
            sgl_end();
        }

        // stroke モード: ラインストリップ
        if (internal::strokeEnabled && n >= 2) {
            sgl_c4f(internal::currentR, internal::currentG, internal::currentB, internal::currentA);
            sgl_begin_line_strip();
            for (size_t i = 0; i < n; i++) {
                sgl_v3f(vertices_[i].x, vertices_[i].y, vertices_[i].z);
            }
            if (closed_ && n > 2) {
                sgl_v3f(vertices_[0].x, vertices_[0].y, vertices_[0].z);
            }
            sgl_end();
        }
    }

    // バウンディングボックス取得
    void getBoundingBox(float& minX, float& minY, float& maxX, float& maxY) const {
        if (vertices_.empty()) {
            minX = minY = maxX = maxY = 0;
            return;
        }
        minX = maxX = vertices_[0].x;
        minY = maxY = vertices_[0].y;
        for (const auto& v : vertices_) {
            if (v.x < minX) minX = v.x;
            if (v.x > maxX) maxX = v.x;
            if (v.y < minY) minY = v.y;
            if (v.y > maxY) maxY = v.y;
        }
    }

    // 長さを計算（ライン長）
    float getPerimeter() const {
        if (vertices_.size() < 2) return 0;
        float len = 0;
        for (size_t i = 1; i < vertices_.size(); i++) {
            float dx = vertices_[i].x - vertices_[i-1].x;
            float dy = vertices_[i].y - vertices_[i-1].y;
            float dz = vertices_[i].z - vertices_[i-1].z;
            len += sqrt(dx*dx + dy*dy + dz*dz);
        }
        if (closed_ && vertices_.size() > 2) {
            float dx = vertices_[0].x - vertices_.back().x;
            float dy = vertices_[0].y - vertices_.back().y;
            float dz = vertices_[0].z - vertices_.back().z;
            len += sqrt(dx*dx + dy*dy + dz*dz);
        }
        return len;
    }

private:
    std::vector<Vec3> vertices_;
    bool closed_;
};

} // namespace trussc
