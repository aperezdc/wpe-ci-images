FROM archlinux/base

RUN pacman --noconfirm --needed -Suy \
coreutils \
fakechroot \
fakeroot \
git \
gnupg \
jq \
lftp \
mcpp \
openssh \
pigz \
reflector \
skopeo \
tar \
zsh \
&& rm -rf /var/cache/pacman/pkg/* \
&& mkdir -p /src

WORKDIR /src/
COPY --chown=nobody:nobody . .
