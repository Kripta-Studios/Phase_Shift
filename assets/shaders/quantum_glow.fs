#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform float time;
uniform vec2 resolution;

out vec4 finalColor;

/* === BLOOM: Extract bright pixels and blur === */
vec3 bloom_extract(vec2 uv) {
    vec3 col = texture(texture0, uv).rgb;
    float brightness = dot(col, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 0.45) return col * (brightness - 0.45) * 2.0;
    return vec3(0.0);
}

vec3 bloom_blur(vec2 uv) {
    vec3 result = vec3(0.0);
    float px = 1.0 / resolution.x;
    float py = 1.0 / resolution.y;
    
    /* 9-tap Gaussian blur */
    float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
    
    result += bloom_extract(uv) * weights[0];
    for (int i = 1; i < 5; i++) {
        float offset = float(i) * 2.0;
        result += bloom_extract(uv + vec2(px * offset, 0.0)) * weights[i];
        result += bloom_extract(uv - vec2(px * offset, 0.0)) * weights[i];
        result += bloom_extract(uv + vec2(0.0, py * offset)) * weights[i];
        result += bloom_extract(uv - vec2(0.0, py * offset)) * weights[i];
    }
    
    return result;
}

void main() {
    vec2 uv = fragTexCoord;
    
    /* === CHROMATIC ABERRATION === */
    float aberration = 0.0015 + sin(time * 0.5) * 0.0003;
    float r = texture(texture0, uv + vec2( aberration, 0.0)).r;
    float g = texture(texture0, uv).g;
    float b = texture(texture0, uv + vec2(-aberration, 0.0)).b;
    vec3 color = vec3(r, g, b);
    
    /* === BLOOM === */
    vec3 glow = bloom_blur(uv);
    color += glow * 0.6;
    
    /* === SCANLINES (CRT) === */
    float scanline = sin(uv.y * resolution.y * 1.5 + time * 2.0) * 0.5 + 0.5;
    scanline = mix(1.0, scanline, 0.06);
    color *= scanline;
    
    /* Fine horizontal lines */
    float fine_line = mod(gl_FragCoord.y, 3.0) < 1.0 ? 0.95 : 1.0;
    color *= fine_line;
    
    /* === VIGNETTE === */
    vec2 vig_uv = uv * (1.0 - uv.yx);
    float vig = vig_uv.x * vig_uv.y * 20.0;
    vig = clamp(pow(vig, 0.35), 0.0, 1.0);
    color *= vig;
    
    /* === COLOR GRADING: Sci-fi cyan/blue tint === */
    color.r *= 0.92;
    color.g *= 1.02;
    color.b *= 1.12;
    
    /* Subtle noise grain for that digital feel */
    float noise = fract(sin(dot(uv * time, vec2(12.9898, 78.233))) * 43758.5453);
    color += (noise - 0.5) * 0.015;
    
    /* === SLIGHT CRT CURVATURE === */
    /* Already applied via color, just ensure no extreme clipping */
    color = clamp(color, 0.0, 1.0);
    
    finalColor = vec4(color, 1.0);
}
