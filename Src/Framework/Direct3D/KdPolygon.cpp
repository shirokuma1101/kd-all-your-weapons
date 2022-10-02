#include "KdPolygon.h"

void KdPolygon::Load(std::string_view filename)
{
	if (!m_spMaterial) { m_spMaterial = std::make_shared<KdMaterial>(); }

	m_spMaterial->Load(filename.data());
}