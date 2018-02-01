#version 460 core

layout (location = 0) out unsigned short z_value;

in GS_FS_INTERFACE
{
    float depth;
} fs_in;

void main()
{
    z_value = unsigned short(1000.0*fs_in.depth);
}