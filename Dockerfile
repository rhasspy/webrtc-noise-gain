FROM quay.io/pypa/manylinux_2_28_x86_64 as build-amd64

FROM quay.io/pypa/manylinux_2_28_aarch64 as build-arm64

ARG TARGETARCH
ARG TARGETVARIANT
FROM build-${TARGETARCH}${TARGETVARIANT} as build
ARG TARGETARCH
ARG TARGETVARIANT

ENV LANG C.UTF-8
ENV DEBIAN_FRONTEND=noninteractive

RUN curl -L 'http://freedesktop.org/software/pulseaudio/webrtc-audio-processing/webrtc-audio-processing-1.3.tar.gz' | \
    tar -xzf - && \
    mv webrtc-audio-processing* /build

WORKDIR /build
RUN python3.11 -m venv .venv && \
    source .venv/bin/activate && \
    pip3 install --upgrade pip && \
    pip3 install --upgrade setuptools wheel && \
    pip3 install meson ninja && \
    meson . build -Dprefix=$PWD/install && \
    ninja -C build && \
    ninja -C build install

COPY ./ ./
RUN for version in 7 8 9 10 11; do \
    "python3.${version}" -m venv ".venv_${version}" && \
    source ".venv_${version}"/bin/activate && \
    pip3 install --upgrade pip && \
    pip3 install --upgrade setuptools wheel && \
    pip3 install pybind11 && \
    python3 setup.py bdist_wheel; \
    done

RUN export so_path="$(find install -name 'libwebrtc-audio-processing-1.so*' | head -n 1)" && \
    export lib_dir="$(dirname "${so_path}")" && \
    export lib_dir="$(realpath "${lib_dir}")" && \
    export LD_LIBRARY_PATH="${lib_dir}:${LD_LIBRARY_PATH}" && \
    auditwheel repair dist/*.whl

WORKDIR /test
RUN for version in 7 8 9 10 11; do \
    "python3.${version}" -m venv ".venv_${version}" && \
    source ".venv_${version}"/bin/activate && \
    pip3 install --upgrade pip && \
    pip3 install --upgrade setuptools wheel && \
    pip3 install webrtc-noise-gain -f /build/wheelhouse/ && \
    python3 /build/test.py; \
    done

# -----------------------------------------------------------------------------

FROM scratch
ARG TARGETARCH
ARG TARGETVARIANT

COPY --from=build /build/wheelhouse/ ./
