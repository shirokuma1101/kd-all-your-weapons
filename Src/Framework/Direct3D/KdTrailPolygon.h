#pragma once

class KdTrailPolygon : public KdPolygon
{
public:
    enum class Trail_Pattern
    {
        eDefault,
        eBillboard,
        eVertices
    };

    void GenerateVertices(std::vector<Vertex>& vertices) const override;

    // 先頭のポイントを取得
    inline Math::Matrix* GetTopPoint()
    {
        if (m_pointList.size() == 0) { return nullptr; }
        return &m_pointList.front();
    }

    // ポイントを追加
    inline void AddPoint(const Math::Matrix& mat)
    {
        m_pointList.push_front(mat);
    }

    // 最後尾のポイントを削除
    inline void DelPoint_Back()
    {
        m_pointList.pop_back();
    }

    // リストの数を取得
    inline int GetNumPoints() const
    {
        return (int)m_pointList.size();
    }

    // 情報（画像・描画パターン・幅）のセット
    void SetInfo(const std::shared_ptr<DirectX11Texture>& tex = nullptr, Trail_Pattern pattern = Trail_Pattern::eDefault) {
        SetTexture(tex);
        SetPattern(pattern);
    }

    inline void SetPattern(Trail_Pattern pattern) { m_pattern = pattern; }

private:

    // 通常描画頂点リストの作成
    void CreateWithDefaultPattern(std::vector<Vertex>& vertex) const;

    // ビルボード描画
    void CreateWithBillboardPattern(std::vector<Vertex>& vertex) const;

    // 頂点情報をそのまま繋げてポリゴンを作成
    void CreateWithVerticesPattern(std::vector<Vertex>& vertex) const;

    // 軌跡の位置などを記憶する行列配列
    std::deque<Math::Matrix>	m_pointList;

    Trail_Pattern m_pattern = Trail_Pattern::eDefault;

    bool m_enable = true;
};
