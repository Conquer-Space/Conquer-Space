#import bevy_pbr::forward_io::VertexOutput

@group(#{MATERIAL_BIND_GROUP}) @binding(0) var<uniform> options: vec4<f32>;
@group(#{MATERIAL_BIND_GROUP}) @binding(1) var terrain: texture_2d<f32>;
@group(#{MATERIAL_BIND_GROUP}) @binding(2) var terrain_sampler: sampler;
@group(#{MATERIAL_BIND_GROUP}) @binding(3) var political: texture_2d<f32>;
@group(#{MATERIAL_BIND_GROUP}) @binding(4) var political_sampler: sampler;

@fragment
fn fragment(mesh: VertexOutput) -> @location(0) vec4<f32> {
    let terrain_color = textureSample(terrain, terrain_sampler, mesh.uv).rgb;
    // The legacy texture encodes water as saturated blue; interpret that mask
    // in the shader, leaving the original source image untouched.
    let water = smoothstep(0.5, 0.9, terrain_color.b) * (1.0 - smoothstep(0.03, 0.1, terrain_color.r + terrain_color.g));
    let surface = mix(terrain_color, vec3<f32>(0.012, 0.045, 0.11), water);
    let tint = textureSample(political, political_sampler, mesh.uv);
    let normal = normalize(mesh.world_normal);
    let light = 0.45 + 0.55 * max(dot(normal, normalize(vec3<f32>(0.4, 0.8, 0.7))), 0.0);
    let color = mix(surface, tint.rgb, tint.a * options.x);
    return vec4<f32>(color * light, 1.0);
}
