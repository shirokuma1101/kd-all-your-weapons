#include "KdSquarePolygon.h" 


KdSquarePolygon::KdSquarePolygon(
    const Math::Vector2* _pScale, const Math::Color* _pColor, const Math::Rectangle* _pRect)
{
    // 4角形ポリゴン用の4頂点を確保
    m_vertSquare.resize(4);

    // 描画の幅・高さの初期化
    if (_pScale)
    {
        SetScale(*_pScale);
    }
    else
    {
        InitScale();
    }

    // 指定があればカラーの初期化
    if (_pColor)
    {
        SetColor(*_pColor);
    }

    // テクスチャ内の描画エリア
    if (_pRect)
    {
        SetUVRect(*_pRect);
    }
    else
    {
        InitRect();
    }
}

void KdSquarePolygon::GenerateVertices(std::vector<Vertex>& vertices) const
{
    vertices.resize(4);

    vertices = m_vertSquare;
}

// 描画の幅と高さの設定
void KdSquarePolygon::SetScale( const Math::Vector2& _scale )
{
    float halfX = _scale.x * 0.5f;
    float halfY = _scale.y * 0.5f;

    // 頂点座標
    m_vertSquare[0].pos = { -halfX, -halfY, 0.0f };	// 左上
    m_vertSquare[1].pos = { -halfX,  halfY, 0.0f };	// 左下
    m_vertSquare[2].pos = {  halfX, -halfY, 0.0f };	// 右上
    m_vertSquare[3].pos = {  halfX,  halfY, 0.0f };	// 右下
}

// テクスチャの描画合成色の設定
void KdSquarePolygon::SetColor(const Math::Color& color)
{
    unsigned int col = 0;
    unsigned char r = static_cast<unsigned int>(color.R() * 255);
    unsigned char g = static_cast<unsigned int>(color.G() * 255);
    unsigned char b = static_cast<unsigned int>(color.B() * 255);
    unsigned char a = static_cast<unsigned int>(color.A() * 255);

    col = (a << 24) | (b << 16) | (g << 8) | r;

    //color.BGRA

    // 頂点カラーの設定
    for (auto& vertex : m_vertSquare)
    {
        vertex.color = col;
    }
}

// テクスチャ内の描画エリアの設定 分割フレームから
void KdSquarePolygon::SetUVRect(int FrameCnt)
{
    // マス座標
    int x = FrameCnt % m_splitX;
    int y = FrameCnt / m_splitX;

    float w = 1.0f / m_splitX;
    float h = 1.0f / m_splitY;

    Math::Vector2 uvMin, uvMax;

    uvMin.x = x * w;
    uvMin.y = y * h;

    uvMax.x = uvMin.x + w;
    uvMax.y = uvMin.y + h;

    SetUVRect(uvMin, uvMax);
}

// テクスチャ内の描画エリアの設定
void KdSquarePolygon::SetUVRect(const Math::Rectangle& _rect)
{
    if (!m_spMaterial || !m_spMaterial->BaseColorTex) { return; }

    Math::Vector2 uvMin, uvMax;
    DirectX11Texture::ConvertRectToUV(*m_spMaterial->BaseColorTex, _rect, &uvMin, &uvMax);

    // UV座標
    SetUVRect(uvMin, uvMax);
}

void KdSquarePolygon::SetUVRect(const Math::Vector2& _uvMin, const Math::Vector2& _uvMax)
{
    // UV座標
    m_vertSquare[0].UV = { _uvMin.x, _uvMax.y };
    m_vertSquare[1].UV = { _uvMin.x, _uvMin.y };
    m_vertSquare[2].UV = { _uvMax.x, _uvMax.y };
    m_vertSquare[3].UV = { _uvMax.x, _uvMin.y };
}

void KdSquarePolygon::SetInfo(const Math::Vector2* pScale, const Math::Color* pColor, const Math::Rectangle* pRect)
{
    // 描画の幅・高さの初期化
    if (pScale)
    {
        SetScale(*pScale);
    }

    // 指定があればカラーの初期化
    if (pColor)
    {
        SetColor(*pColor);
    }

    // テクスチャ内の描画エリア
    if (pRect)
    {
        SetUVRect(*pRect);
    }
}

void KdSquarePolygon::InitScale()
{
    // 頂点座標
    m_vertSquare[0].pos = { -0.5f, -0.5f, 0 };
    m_vertSquare[1].pos = { -0.5f,  0.5f, 0 };
    m_vertSquare[2].pos = {  0.5f, -0.5f, 0 };
    m_vertSquare[3].pos = {  0.5f,  0.5f, 0 };
}

void KdSquarePolygon::InitRect()
{
    // UV座標
    m_vertSquare[0].UV = { 0, 1 };
    m_vertSquare[1].UV = { 0, 0 };
    m_vertSquare[2].UV = { 1, 1 };
    m_vertSquare[3].UV = { 1, 0 };
}
