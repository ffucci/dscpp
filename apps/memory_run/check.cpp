#include <iostream>
#include <memory>

int main() {
    std::cout << "GCC Version: " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << std::endl;
#ifdef __cpp_lib_allocate_at_least
    std::cout << "Feature Macro Found!" << std::endl;
#else
    std::cout << "Feature Macro NOT Found." << std::endl;
#endif
    return 0;
}