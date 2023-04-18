cd ../Lab1 \
&& ./generate.sh \
&& cd ../Lab2 \
&& mkdir -p build \
&& cmake . -Bbuild \
&& cd build \
&& make