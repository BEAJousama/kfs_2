# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|
  config.vm.box = "ubuntu/focal64"
  
  config.vm.provider "virtualbox" do |vb|
    vb.gui = true
    vb.name = "KFS2-VM"
    vb.memory = "4096"
    vb.cpus = 3
    vb.customize ["modifyvm", :id, "--accelerate3d", "on"]
    vb.customize ["modifyvm", :id, "--vram", "128"]
    vb.customize ["modifyvm", :id, "--clipboard", "bidirectional"]
    vb.customize ["modifyvm", :id, "--draganddrop", "bidirectional"]
  end
  
  # First: sync to vagrant user (this always works)
  config.vm.synced_folder "/Users/obeaj/Desktop/kfs_2", "/home/vagrant/kfs_shared"
  
  config.vm.provision "shell", inline: <<-SHELL
    apt-get update
    apt-get install -y ubuntu-desktop-minimal
    apt-get install -y virtualbox-guest-dkms virtualbox-guest-utils virtualbox-guest-x11
    systemctl set-default graphical.target
    
    # Create obeaj user
    useradd -m -s /bin/bash obeaj
    echo "obeaj:obeaj" | chpasswd
    usermod -aG sudo obeaj
    
    # Create bind mount from vagrant shared folder to obeaj home
    mkdir -p /home/obeaj/kfs_2
    mount --bind /home/vagrant/kfs_shared /home/obeaj/kfs_2
    
    # Make bind mount persistent across reboots
    echo "/home/vagrant/kfs_shared /home/obeaj/kfs_2 none bind 0 0" >> /etc/fstab
    
    # Fix ownership and permissions
    chown -R obeaj:obeaj /home/obeaj/kfs_2
    
    # Give obeaj access to vagrant group for file operations
    usermod -aG vagrant obeaj
    
    apt-get install -y \
      build-essential \
      bison \
      flex \
      libgmp3-dev \
      libmpc-dev \
      libmpfr-dev \
      texinfo \
      libisl-dev \
      qemu-system \
      grub-pc-bin \
      xorriso \
      nasm \
      curl \
      gcc \
      g++ \
      make
  SHELL
  
  config.vm.network "private_network", ip: "192.168.56.10"
  config.vm.network "forwarded_port", guest: 80, host: 8080
  config.vm.network "forwarded_port", guest: 3000, host: 3000
end