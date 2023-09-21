import platform
from pathlib import Path

# Available at setup time due to pyproject.toml
from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup

_DIR = Path(__file__).parent
_INSTALL_DIR = _DIR / "install"

__version__ = "0.0.2"

lib_dir = next(f.parent for f in _INSTALL_DIR.rglob("*.so*"))

ext_modules = [
    Pybind11Extension(
        "webrtc_noise_gain_cpp",
        [
            "python.cpp",
        ],
        define_macros=[("VERSION_INFO", __version__)],
        include_dirs=[
            str(_INSTALL_DIR / "include"),
            str(_INSTALL_DIR / "include" / "webrtc-audio-processing-1"),
        ],
        library_dirs=[str(lib_dir)],
        libraries=["webrtc-audio-processing-1", "webrtc-audio-coding-1"],
    ),
]

setup(
    name="webrtc_noise_gain",
    version=__version__,
    author="Michael Hansen",
    author_email="mike@rhasspy.org",
    url="https://github.com/rhasspy/webrtc-noise-gain",
    description="Noise suppression and automatic gain with webrtc",
    long_description="",
    packages=["webrtc_noise_gain"],
    install_requires=["numpy<2"],
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.7",
)
