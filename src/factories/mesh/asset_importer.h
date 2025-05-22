#pragma once

#include "geometry/static_mesh.h"


namespace GLT::factory::geometry {

    bool load_mesh(const std::filesystem::path& file_path, ref<GLT::geometry::static_mesh> out_mesh);

}
