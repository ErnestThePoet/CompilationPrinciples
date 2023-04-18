cd ../Lab1 \
&& ./generate.sh \
&& cd ../Lab3 \
&& mkdir -p build \
&& cmake . -Bbuild \
&& cd build \
&& make