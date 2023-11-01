/*
TODO: Adapt this to the new OOP system
*/
// #ifndef GLPY_POSTP_HPP
// #define GLPY_POSTP_HPP
// #include"gsdl.hpp"
// namespace pygame{
//     using Kernal = glm::mat3;
//     inline Shader krnl;
//     inline void ppInit(){
//         krnl.program = loadprogram(u8"rsrc/2d_textured_vertex.glsl"sv,u8"rsrc/kernalfs.glsl"sv);
//     }
//     inline void ppApply(const zTexture& src, const Framebuffer& dst, Shader& shd=texture_shader, float sz=1.0f){
//         dst.bind();
//         blit(src,{0.0f,0.0f},sz,0.0f,{0.0f,0.0f,1.0f,1.0f},shd,false);
//     }
//     inline Kernal edgeDet{
//         1,1,1,
//         1,-8,1,
//         1,1,1
//     };
//     inline float size_fit(float w, float h,float dw=1920.0f, float dh=1080.0f){
//         float wmul = dw/w;
//         float hmul = dh/h;
//         return glm::min(wmul,hmul);
//     }
//     class Scene{
//         Framebuffer fb;
//         mutable Framebuffer tmp;
//         mutable Renderbuffer dep;
//         sTexture _graphics;
//         zTexture graphics;
//         sTexture _tmphics;
//         zTexture tmphics;
//         GLsizei w;
//         GLsizei h;
//         mutable bool redrew = true;
//         float sz;
//         public:
//             Scene(GLsizei wid, GLsizei hgt,float scaleup=1.0f);
//             void bind(bool threed=false) const{
//                 gl_call(glViewport,0,0,w,h);
//                 fb.bind();
//                 if(threed){
//                     gl_call(glEnable,GL_DEPTH_TEST);
//                     gl_call(glClear,GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
//                 }else{
//                     gl_call(glDisable,GL_DEPTH_TEST);
//                     gl_call(glClear,GL_COLOR_BUFFER_BIT);
//                 }
//                 redrew = true;
//             }
//             void applyKernel(const Kernal& k,GLfloat off=1.0f/300.0f){
//                 gl_call(glDisable,GL_DEPTH_TEST);
//                 krnl.um3("kernel",k);
//                 krnl.u1f("offset",off);
//                 ppApply(redrew?graphics:tmphics,tmp,krnl,size_fit(w,h));
//                 redrew = false;
//             }
//             void alpha(float a){
//                 graphics.alpha() = a;
//             }
//             void draw(Point pos={0.0f, 0.0f},GLsizei dw=0, GLsizei dh=0) const{
//                 gl_call(glDisable,GL_DEPTH_TEST);
//                 Framebuffer::unbind();
//                 if(dw!=0&&dh!=0){
//                     gl_call(glViewport,0,0,dw,dh);
//                 }else if(display::gl_context){
//                     display::gl_context->restore_viewport();
//                 }
//                 gl_call(glClear,GL_COLOR_BUFFER_BIT);
//                 blit(tmphics,pos,sz,0.0f,{0.0f,0.0f,1.0f,1.0f},texture_shader,false);
//             }
//             Scene(const Scene&) = delete;
//     };
// }
// #endif
