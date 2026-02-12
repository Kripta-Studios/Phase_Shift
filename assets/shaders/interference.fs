#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Custom uniforms
uniform float time;
uniform vec2 resolution;

// Output fragment color
out vec4 finalColor;

// Simplex noise or sine wave interference pattern
void main()
{
    vec2 st = fragTexCoord;
    vec4 texColor = texture(texture0, st);
    
    // Wave parameters
    float frequency = 20.0;
    float speed = 3.0;
    float amplitude = 0.05;
    
    // Calculate wave displacement
    float wave = sin(st.y * frequency + time * speed) * amplitude;
    
    // Distort texture coordinates
    vec2 distortedSt = vec2(st.x + wave, st.y);
    
    // Sample texture with distorted coordinates
    // We mix it with original to create a "double image" interference effect
    vec4 color1 = texture(texture0, st);
    vec4 color2 = texture(texture0, distortedSt);
    
    // Construct interference pattern
    // Constructive/Destructive interference visual
    float interference = (sin(st.x * 30.0 + time * 2.0) + 1.0) * 0.5;
    
    vec4 final = mix(color1, color2, 0.5);
    
    // Add some color shift based on interference
    final.r += interference * 0.2;
    final.b += (1.0 - interference) * 0.2;
    
    finalColor = final * colDiffuse * fragColor;
}
