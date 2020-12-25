package keys;

import org.objectweb.asm.*;

import java.lang.instrument.*;
import java.security.ProtectionDomain;

import static org.objectweb.asm.Opcodes.ASM9;

public class Agent {
	
	public static void premain(String arg, Instrumentation inst) {
		inst.addTransformer(new ClassTransformer());
	}
	
	public static class ClassTransformer implements ClassFileTransformer {
		
		@Override
		public byte[] transform(ClassLoader loader, String className, Class<?> classBeingRedefined, ProtectionDomain protectionDomain, byte[] basicClass) {
			System.out.println("Loading class " + className.replaceAll("/", "."));
			
			if (className.equals("keys/ReadKey")) {
				ClassReader cr = new ClassReader(basicClass);
				ClassWriter cw = new ClassWriter(ClassWriter.COMPUTE_MAXS | ClassWriter.COMPUTE_FRAMES);
				ClassVisitor cv = new ReadKeyClassVisitor(cw);
				cr.accept(cv, ClassReader.EXPAND_FRAMES);
				return cw.toByteArray();
			}
			
			return basicClass;
		}
		
		public static class ReadKeyClassVisitor extends ClassVisitor {
			
			public ReadKeyClassVisitor(ClassVisitor classVisitor) {
				super(ASM9, classVisitor);
			}
			
			@Override
			public MethodVisitor visitMethod(int access, String name, String descriptor, String signature, String[] exceptions) {
				MethodVisitor mv = super.visitMethod(access, name, descriptor, signature, exceptions);
				
				if (name.equals("readKeys")) {
					return new ReadKeyReadKeysMethodVisitor(mv);
				}
				
				return mv;
			}
			
			public static class ReadKeyReadKeysMethodVisitor extends MethodVisitor {
				
				public ReadKeyReadKeysMethodVisitor(MethodVisitor methodVisitor) {
					super(ASM9, methodVisitor);
				}
				
				@Override
				public void visitLdcInsn(Object value) {
					if ("KeyPair.txt".equals(value))
						value = "KeyPairHack.txt";
					
					super.visitLdcInsn(value);
				}
			}
		}
	}
}
