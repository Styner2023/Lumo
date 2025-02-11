#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <ImGuiPack.h>

class Animator : public ImSequencer::SequenceInterface {
public:
    struct AnimatorItem {
        int mType;
        int mFrameStart, mFrameEnd;
        bool mExpanded;
    };

public:
    std::array<std::string, 1U> m_ItemTypes = {"Frame"};

    int32_t m_StartFrame = 0;
    int32_t m_EndFrame = 180;
    int32_t m_CurrentFrame = 0;
    std::vector<AnimatorItem> myItems = {{0, 0, 180, true}};

public:
    int GetFrameMin() const override {
        return m_StartFrame;
    }

    int GetFrameMax() const override {
        return m_EndFrame;
    }

    int GetItemCount() const override {
        return (int)myItems.size();
    }

    int GetItemTypeCount() const override {
        return sizeof(m_ItemTypes) / sizeof(char*);
    }

    const char* GetItemTypeName(int typeIndex) const override {
        return m_ItemTypes[typeIndex].c_str();
    }

    const char* GetItemLabel(int index) const override {
        static char tmps[512];
        snprintf(tmps, 512, "[%02d] %s", index, GetItemTypeName(myItems[index].mType));
        return tmps;
    }

    void Get(int index, int** start, int** end, int* type, unsigned int* color) override {
        AnimatorItem& item = myItems[index];
        if (color)
            *color = 0xFFAA8080;  // same color for everyone, return color based on type
        if (start)
            *start = &item.mFrameStart;
        if (end)
            *end = &item.mFrameEnd;
        if (type)
            *type = item.mType;
    }

    void Add(int type) override {
        myItems.push_back(AnimatorItem{type, 0, 10, false});
    };

    void Del(int index) override {
        myItems.erase(myItems.begin() + index);
    }

    void Duplicate(int index) override {
        myItems.push_back(myItems[index]);
    }

    size_t GetCustomHeight(int index) override {
        return myItems[index].mExpanded ? 300 : 0;
    }

    void DoubleClick(int index) override {
        if (myItems[index].mExpanded) {
            myItems[index].mExpanded = false;
            return;
        }
        for (auto& item : myItems)
            item.mExpanded = false;
        myItems[index].mExpanded = !myItems[index].mExpanded;
    }
};

class ProjectFile;
class AnimatePane : public AbstractPane {
private:
    Animator m_Animator;

public:
    bool Init() override;
    void Unit() override;
    bool DrawWidgets(const uint32_t& vCurrentFrame, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawOverlays(
        const uint32_t& vCurrentFrame, const ImRect& vRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawPanes(
        const uint32_t& vCurrentFrame, bool* vOpened, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;
    bool DrawDialogsAndPopups(
        const uint32_t& vCurrentFrame, const ImRect& vMaxRect, ImGuiContext* vContextPtr = nullptr, void* vUserDatas = nullptr) override;

public:  // singleton
    static std::shared_ptr<AnimatePane> Instance() {
        static std::shared_ptr<AnimatePane> _instance = std::make_shared<AnimatePane>();
        return _instance;
    }

public:
    AnimatePane();                              // Prevent construction
    AnimatePane(const AnimatePane&) = default;  // Prevent construction by copying
    AnimatePane& operator=(const AnimatePane&) {
        return *this;
    };                       // Prevent assignment
    virtual ~AnimatePane();  // Prevent unwanted destruction};
};
