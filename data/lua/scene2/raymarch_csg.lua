--[[ raymarch_csg.lua

    Takes some basic raymarching code from shadertoy and applies the incoming
    view and projection matrices so raymarched objects can coexist with
    rasterized ones by sharing a color and depth buffer.
]]
raymarch_csg = {}

raymarch_csg.__index = raymarch_csg

function raymarch_csg.new(...)
    local self = setmetatable({}, raymarch_csg)
    if self.init ~= nil and type(self.init) == "function" then
        self:init(...)
    end 
    return self
end

function raymarch_csg:init()
    self.shader = nil
end

local ffi = require("ffi")
require("util.fullscreen_shader")

local glIntv = ffi.typeof('GLint[?]')
local glUintv = ffi.typeof('GLuint[?]')
local glFloatv = ffi.typeof('GLfloat[?]')

local rm_frag = [[
uniform mat4 mvmtx;
uniform mat4 prmtx;

// Simple ray marching example
// @var url https://www.shadertoy.com/view/ldB3Rw
// @var author gltracy
// @var license CC BY-NC-SA 3.0

// @var headSize 6.0
// @var eyePos -2.5952096 5.4259381 -20.277588

const int max_iterations = 255;
const float stop_threshold = 0.001;
const float grad_step = 0.1;
const float clip_far = 1000.0;

// math
const float PI = 3.14159265359;
const float DEG_TO_RAD = PI / 180.0;

// distance function
float dist_sphere( vec3 pos, float r ) {
    return length( pos ) - r;
}

float dist_box( vec3 pos, vec3 size ) {
    return length( max( abs( pos ) - size, 0.0 ) );
}

float dist_box( vec3 v, vec3 size, float r ) {
    return length( max( abs( v ) - size, 0.0 ) ) - r;
}

// get distance in the world
float dist_field( vec3 pos ) {
    // ...add objects here...

    // floor
    float thick = 0.00001;
    float d2 = dist_box( pos + vec3( 0.0, 0.0, 0.0 ), vec3( 10.0, thick, 10.0 ), 0.05 );
    float d3 = dist_box( pos + vec3( 0.0, -3.0, 0.0 ), vec3( 10.0, thick, 10.0 ), 0.05 );
    d2 = min( d2, d3 );
#if 0
    // Manually tease out translation component
    vec3 trans = obmtx[3].xyz;
    pos -= trans;

    pos.y -= 1.5;
    // Multiplying this in reverse order(vec*mtx) is equivalent to transposing the matrix.
    pos = vec4( vec4(pos,1.0) * obmtx ).xyz;

    // Manually tease out scale component
    float scale = length(obmtx[0].xyz);
    pos /= scale * scale;
#endif
    // object 0 : sphere
    float d0 = dist_sphere( pos, 0.5*1.35 );

    // object 1 : cube
    float d1 = dist_box( pos, vec3( 0.5*1.0 ) );

    // union     : min( d0,  d1 )
    // intersect : max( d0,  d1 )
    // subtract  : max( d1, -d0 )
    return max( d1, -d0 );
    //return min( d2, max( d1, -d0 ) );
}

// phong shading
vec3 shading( vec3 v, vec3 n, vec3 eye ) {
    // ...add lights here...
    float shininess = 16.0;
    vec3 final = vec3( 0.0 );

    vec3 ev = normalize( v - eye );
    vec3 ref_ev = reflect( ev, n );

    // light 0
    {
        vec3 light_pos   = vec3( 20.0, 20.0, 20.0 );
        vec3 light_color = vec3( 1.0, 0.7, 0.7 );

        vec3 vl = normalize( light_pos - v );

        float diffuse  = max( 0.0, dot( vl, n ) );
        float specular = max( 0.0, dot( vl, ref_ev ) );
        specular = pow( specular, shininess );

        final += light_color * ( diffuse + specular );
    }

    // light 1
    {
        vec3 light_pos   = vec3( -20.0, -20.0, -20.0 );
        vec3 light_color = vec3( 0.3, 0.7, 1.0 );

        vec3 vl = normalize( light_pos - v );

        float diffuse  = max( 0.0, dot( vl, n ) );
        float specular = max( 0.0, dot( vl, ref_ev ) );
        specular = pow( specular, shininess );

        final += light_color * ( diffuse + specular );
    }

    return final;
}

// get gradient in the world
vec3 gradient( vec3 pos ) {
    const vec3 dx = vec3( grad_step, 0.0, 0.0 );
    const vec3 dy = vec3( 0.0, grad_step, 0.0 );
    const vec3 dz = vec3( 0.0, 0.0, grad_step );
    return normalize (
        vec3(
            dist_field( pos + dx ) - dist_field( pos - dx ),
            dist_field( pos + dy ) - dist_field( pos - dy ),
            dist_field( pos + dz ) - dist_field( pos - dz )
        )
    );
}

// ray marching
float ray_marching( vec3 origin, vec3 dir, float start, float end ) {
    float depth = start;
    for ( int i = 0; i < max_iterations; i++ ) {
        float dist = dist_field( origin + dir * depth );
        if ( dist < stop_threshold ) {
            return depth;
        }
        depth += dist;
        if ( depth >= end) {
            return end;
        }
    }
    return end;
}

vec3 getSceneColor( in vec3 ro, in vec3 rd, inout float depth )
{
    // ray marching
    depth = ray_marching( ro, rd, 0.0, clip_far );
    if ( depth >= clip_far ) {
        discard;
    }

    // shading
    vec3 pos = ro + rd * depth;
    vec3 n = gradient( pos );
    vec3 col = shading( pos, n, ro );

    return col;
}

///////////////////////////////////////////////////////////////////////////////
// Patch in the Rift's heading to raymarch shader writing out color and depth.
// http://blog.hvidtfeldts.net/

// Translate the origin to the camera's location in world space.
vec3 getEyePoint(mat4 mvmtx)
{
    vec3 ro = -mvmtx[3].xyz;
    return ro;
}

// Construct the usual eye ray frustum oriented down the negative z axis.
// http://antongerdelan.net/opengl/raycasting.html
vec3 getRayDirection(vec2 uv)
{
    vec4 ray_clip = vec4(uv.x, uv.y, -1., 1.);
    vec4 ray_eye = inverse(prmtx) * ray_clip;
    return normalize(vec3(ray_eye.x, ray_eye.y, -1.));
}

void main()
{
    vec2 uv11 = uv * 2.0 - vec2(1.0);
    vec3 ro = getEyePoint(mvmtx);
    vec3 rd = getRayDirection(uv11);

    ro *= mat3(mvmtx);
    rd *= mat3(mvmtx);

    float depth = 9999.0;
    vec3 col = getSceneColor(ro, rd, depth);

    // Write to depth buffer
    vec3 eyeFwd = vec3(0.,0.,-1.) * mat3(mvmtx);
    float eyeHitZ = -depth * dot(rd, eyeFwd);
    float p10 = prmtx[2].z;
    float p11 = prmtx[3].z;
    // A little bit of algebra...
    float ndcDepth = -p10 + -p11 / eyeHitZ;
    float dep = ((gl_DepthRange.diff * ndcDepth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;

    gl_FragDepth = dep;
    fragColor = vec4(col, 1.0);
}
]]

function raymarch_csg:initGL()
    self.shader = FullscreenShader.new(rm_frag)
    self.shader:initGL()
end

function raymarch_csg:exitGL()
    self.shader:exitGL()
end

function raymarch_csg:render_for_one_eye(view, proj)
    local function set_variables(prog)
        local umv_loc = gl.glGetUniformLocation(prog, "mvmtx")
        gl.glUniformMatrix4fv(umv_loc, 1, GL.GL_FALSE, glFloatv(16, view))
        local upr_loc = gl.glGetUniformLocation(prog, "prmtx")
        gl.glUniformMatrix4fv(upr_loc, 1, GL.GL_FALSE, glFloatv(16, proj))
    end

    self.shader:render(view, proj, set_variables)
end

return raymarch_csg
