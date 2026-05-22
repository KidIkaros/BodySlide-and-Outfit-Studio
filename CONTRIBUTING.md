# Contributing to PolyForge

Thank you for your interest in contributing to PolyForge! This document provides guidelines and instructions for contributing.

## Getting Started

1. Fork the repository on GitHub
2. Clone your fork: `git clone https://github.com/KidIkaros/PolyForge.git`
3. Create a feature branch: `git checkout -b feature/your-feature-name`
4. Make your changes
5. Run tests to ensure nothing is broken
6. Commit your changes with clear, descriptive messages
7. Push to your fork and create a Pull Request

## Code Style

- Follow the existing code style (see `.clang-format`)
- Use meaningful variable and function names
- Add comments for complex logic
- Keep functions focused and small
- Maximum line length: 180 characters

## Commit Messages

Use clear, descriptive commit messages:
- Start with a verb (Add, Fix, Improve, Remove, etc.)
- Be specific about what changed
- Explain the reasoning if relevant

Example:
```
Add EdgeAwareUpsample edge detection threshold

- Use configurable epsilon threshold for robustness
- Prevents artifacts at depth discontinuities
- Improves mesh quality near sharp edges
```

## Testing

All new features should include appropriate tests:
- Unit tests for core functionality
- Test file location: `tests/universal_model/TestUniversalModel.cpp`
- Tests should verify behavior, not just exercise code
- Run tests before submitting PR

```bash
# Build and run tests
mkdir -p build && cd build
cmake ..
make
ctest --output-on-failure
```

## Pull Request Guidelines

- Reference issues when applicable
- Describe what the PR does in the description
- Ensure all tests pass
- Verify code compiles on both Linux and Windows
- Keep PRs focused - one feature or fix per PR

## Areas to Contribute

- **Mesh processing algorithms**: Edge-aware upsampling, hole-filling, mesh optimization
- **Format support**: Add support for new 3D model formats
- **Performance**: Profile and optimize critical paths
- **Testing**: Improve test coverage for core components
- **Documentation**: Improve code and user documentation

## Questions?

Open an issue on GitHub for questions about contributing.