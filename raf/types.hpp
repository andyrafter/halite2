#pragma once

namespace raf {
    /// A poor man's std::optional.
    template<typename T>
    using possibly = std::pair<T, bool>;
}
