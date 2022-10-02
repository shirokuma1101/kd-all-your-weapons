#pragma once

// 四角形ポリゴンクラス
class KdSquarePolygon : public KdPolygon
{
public:

    KdSquarePolygon(const Math::Vector2* _pScale = nullptr, const Math::Color* _pColor = nullptr, const Math::Rectangle* _pRect = nullptr);

    void GenerateVertices(std::vector<Vertex>& vertices) const override;

    // 描画の幅と高さの設定
    void SetScale(const Math::Vector2& _scale);

    // テクスチャの描画合成色の設定
    void SetColor(const Math::Color& _color);

    // テクスチャ内の描画エリアの設定
    void SetUVRect(int FrameCnt);
    void SetUVRect(const Math::Rectangle& _rect);
    void SetUVRect(const Math::Vector2& _minUV, const Math::Vector2& _maxUV);

    void SetInfo(const Math::Vector2* pScale, const Math::Color* pColor = nullptr, const Math::Rectangle* pRect = nullptr);

    // マテリアルをセット
    inline void SetTexture(const std::shared_ptr<DirectX11Texture>& tex)
    {
        if (!m_spMaterial) { m_spMaterial = std::make_shared<KdMaterial>(); }

        m_spMaterial->BaseColorTex = tex;
    }

    // テクスチャの分割数を設定
    inline void SetSplit(UINT splitX, UINT splitY)
    {
        m_splitX = splitX;
        m_splitY = splitY;
    }

    // 描画
    void Draw(const Math::Matrix& mWorld);

    UINT GetSplitX() { return m_splitX; }
    UINT GetSplitY() { return m_splitY; }

    const std::shared_ptr<KdMaterial>& GetMaterial() { return m_spMaterial; }

protected:

    void InitScale();
    void InitRect();

    // 頂点配列
    std::vector<Vertex> m_vertSquare;

    UINT m_splitX = 1;
    UINT m_splitY = 1;
};
