#pragma once

#include "geometry/static_mesh.h"


namespace GLT::factory::geometry {

    void optimize_static_mesh(ref<GLT::geometry::static_mesh> mesh);

    bool load_static_mesh(const std::filesystem::path& file_path, ref<GLT::geometry::static_mesh> out_mesh);

}
