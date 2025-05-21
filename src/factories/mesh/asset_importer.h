#pragma once

#include "world/static_mesh.h"


namespace GLT::factory::mesh {

    bool load_mesh(const std::filesystem::path& file_path, ref<GLT::mesh::static_mesh> out_mesh);

}
