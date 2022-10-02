#pragma once

class KdPolygon
{
public:
    struct Vertex
    {
        Math::Vector3 pos;
        Math::Vector2 UV;
        Math::Vector3 normal = Math::Vector3::Backward;
        unsigned int  color = 0xFFFFFFFF;
        Math::Vector3 tangent = Math::Vector3::Left;
    };

    KdPolygon() {}
    virtual ~KdPolygon(){}

    // 最終的には禁止になる関数
    void Load(std::string_view filename);

    virtual void GenerateVertices(std::vector<Vertex>& vertices) const = 0;

    // マテリアルをセット
    const std::shared_ptr<KdMaterial>& SetMaterial(const std::shared_ptr<KdMaterial>& material)
    {
        m_spMaterial = material;
    }

    inline void SetTexture(const std::shared_ptr<DirectX11Texture>& tex)
    {
        if (!m_spMaterial) { m_spMaterial = std::make_shared<KdMaterial>(); }

        m_spMaterial->BaseColorTex = tex;
    }

    const std::shared_ptr<KdMaterial>& GetMaterial() const { return m_spMaterial; }

    bool IsEnable() const { return (m_enable && m_spMaterial); }
    void SetEnable(bool enable) { m_enable = enable; }


protected:
    std::shared_ptr<KdMaterial> m_spMaterial = nullptr;

    bool m_enable = true;
};
