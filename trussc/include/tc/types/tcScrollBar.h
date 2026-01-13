#pragma once

#include "tcRectNode.h"
#include "tcScrollContainer.h"

namespace trussc {

// =============================================================================
// ScrollBar - Visual scroll indicator for ScrollContainer
//
// A simple scroll bar that syncs with ScrollContainer.
// Renders as a rounded slot shape using stroke with caps.
// Hidden when there's no scroll range.
//
// Usage:
//   auto vBar = make_shared<ScrollBar>(scrollContainer, ScrollBar::Vertical);
//   scrollContainer->addChild(vBar);
//
// =============================================================================

class ScrollBar : public RectNode {
public:
    using Ptr = std::shared_ptr<ScrollBar>;

    enum Direction {
        Vertical,
        Horizontal
    };

    ScrollBar(ScrollContainer* container, Direction dir = Vertical)
        : container_(container), direction_(dir) {

        if (direction_ == Vertical) {
            setSize(barWidth_, 100);  // Height will be updated
        } else {
            setSize(100, barWidth_);  // Width will be updated
        }
    }

    // -------------------------------------------------------------------------
    // Settings
    // -------------------------------------------------------------------------

    Color getBarColor() const { return barColor_; }
    void setBarColor(const Color& color) { barColor_ = color; }

    float getBarWidth() const { return barWidth_; }
    void setBarWidth(float width) {
        barWidth_ = width;
        if (direction_ == Vertical) {
            setWidth(width);
        } else {
            setHeight(width);
        }
        updateFromContainer();  // Recalculate position
    }

    float getMargin() const { return margin_; }
    void setMargin(float margin) {
        margin_ = margin;
        updateFromContainer();
    }

    // Offset for drawing: round(barWidth/2)
    float getOffset() const {
        return std::round(barWidth_ / 2);
    }

    // -------------------------------------------------------------------------
    // Update (call each frame or when scroll changes)
    // -------------------------------------------------------------------------

    void updateFromContainer() {
        if (!container_) {
            isActive = false;
            return;
        }

        if (direction_ == Vertical) {
            updateVertical();
        } else {
            updateHorizontal();
        }
    }

protected:
    ScrollContainer* container_ = nullptr;
    Direction direction_ = Vertical;
    Color barColor_ = Color(1.0f, 1.0f, 1.0f, 0.5f);
    float barWidth_ = 5.0f;
    float margin_ = 2.0f;

    // -------------------------------------------------------------------------
    // Update methods
    // -------------------------------------------------------------------------

    void updateVertical() {
        float maxScroll = container_->getMaxScrollY();
        if (maxScroll <= 0) {
            isActive = false;
            return;
        }
        isActive = true;

        // Calculate bar height based on visible ratio
        auto* content = container_->getContentRect();
        if (!content) {
            isActive = false;
            return;
        }

        float margin = getMargin();
        float containerHeight = container_->getHeight();
        float contentHeight = content->getHeight();
        float visibleRatio = containerHeight / contentHeight;
        float barHeight = (containerHeight - margin * 2) * visibleRatio;
        barHeight = std::max(barWidth_ * 2, barHeight);  // Minimum size

        setSize(barWidth_, barHeight);

        // Calculate bar position
        float scrollRatio = container_->getScrollY() / maxScroll;
        float barY = margin + scrollRatio * (containerHeight - margin * 2 - barHeight);

        // Position: right edge with margin
        float barX = container_->getWidth() - barWidth_ - margin;

        setPos(barX, barY);
    }

    void updateHorizontal() {
        float maxScroll = container_->getMaxScrollX();
        if (maxScroll <= 0) {
            isActive = false;
            return;
        }
        isActive = true;

        auto* content = container_->getContentRect();
        if (!content) {
            isActive = false;
            return;
        }

        float margin = getMargin();
        float containerWidth = container_->getWidth();
        float contentWidth = content->getWidth();
        float visibleRatio = containerWidth / contentWidth;
        float barWidth = (containerWidth - margin * 2) * visibleRatio;
        barWidth = std::max(barWidth_ * 2, barWidth);  // Minimum size

        setSize(barWidth, barWidth_);

        float scrollRatio = container_->getScrollX() / maxScroll;
        float barX = margin + scrollRatio * (containerWidth - margin * 2 - barWidth);

        // Position: bottom edge with margin
        float barY = container_->getHeight() - barWidth_ - margin;

        setPos(barX, barY);
    }

    // -------------------------------------------------------------------------
    // Drawing
    // -------------------------------------------------------------------------

    void draw() override {
        pushStyle();

        setColor(barColor_.r, barColor_.g, barColor_.b, barColor_.a);
        noFill();
        setStrokeWeight(barWidth_);
        setStrokeCap(StrokeCap::Round);

        // Draw as rounded slot using stroke with caps
        float capOffset = barWidth_ / 2;

        beginStroke();
        if (direction_ == Vertical) {
            vertex(barWidth_ / 2, capOffset);
            vertex(barWidth_ / 2, getHeight() - capOffset);
        } else {
            vertex(capOffset, barWidth_ / 2);
            vertex(getWidth() - capOffset, barWidth_ / 2);
        }
        endStroke();

        popStyle();
    }
};

} // namespace trussc
