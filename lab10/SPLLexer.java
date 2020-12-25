// Generated from CPP14.g4 by ANTLR 4.9
import org.antlr.v4.runtime.Lexer;
import org.antlr.v4.runtime.CharStream;
import org.antlr.v4.runtime.Token;
import org.antlr.v4.runtime.TokenStream;
import org.antlr.v4.runtime.*;
import org.antlr.v4.runtime.atn.*;
import org.antlr.v4.runtime.dfa.DFA;
import org.antlr.v4.runtime.misc.*;

@SuppressWarnings({"all", "warnings", "unchecked", "unused", "cast"})
public class SPLLexer extends Lexer {
	static { RuntimeMetaData.checkVersion("4.9", RuntimeMetaData.VERSION); }

	protected static final DFA[] _decisionToDFA;
	protected static final PredictionContextCache _sharedContextCache =
		new PredictionContextCache();
	public static final int
		T__0=1, T__1=2, T__2=3, T__3=4, T__4=5, T__5=6, T__6=7, T__7=8, T__8=9, 
		T__9=10, T__10=11, T__11=12, T__12=13, T__13=14, T__14=15, T__15=16, T__16=17, 
		T__17=18, T__18=19, T__19=20, T__20=21, T__21=22, T__22=23, T__23=24, 
		ID=25, STR=26, INT=27, WS=28;
	public static String[] channelNames = {
		"DEFAULT_TOKEN_CHANNEL", "HIDDEN"
	};

	public static String[] modeNames = {
		"DEFAULT_MODE"
	};

	private static String[] makeRuleNames() {
		return new String[] {
			"T__0", "T__1", "T__2", "T__3", "T__4", "T__5", "T__6", "T__7", "T__8", 
			"T__9", "T__10", "T__11", "T__12", "T__13", "T__14", "T__15", "T__16", 
			"T__17", "T__18", "T__19", "T__20", "T__21", "T__22", "T__23", "ID", 
			"STR", "INT", "WS"
		};
	}
	public static final String[] ruleNames = makeRuleNames();

	private static String[] makeLiteralNames() {
		return new String[] {
			null, "';'", "'variabel'", "'='", "'-'", "'!'", "'/'", "'*'", "'+'", 
			"'<='", "'>='", "'<'", "'>'", "'!='", "'OCH'", "'ELLER'", "'('", "')'", 
			"'skriv'", "'om'", "'annars om'", "'annars'", "'{'", "'}'", "'n\u00E4r'"
		};
	}
	private static final String[] _LITERAL_NAMES = makeLiteralNames();
	private static String[] makeSymbolicNames() {
		return new String[] {
			null, null, null, null, null, null, null, null, null, null, null, null, 
			null, null, null, null, null, null, null, null, null, null, null, null, 
			null, "ID", "STR", "INT", "WS"
		};
	}
	private static final String[] _SYMBOLIC_NAMES = makeSymbolicNames();
	public static final Vocabulary VOCABULARY = new VocabularyImpl(_LITERAL_NAMES, _SYMBOLIC_NAMES);

	/**
	 * @deprecated Use {@link #VOCABULARY} instead.
	 */
	@Deprecated
	public static final String[] tokenNames;
	static {
		tokenNames = new String[_SYMBOLIC_NAMES.length];
		for (int i = 0; i < tokenNames.length; i++) {
			tokenNames[i] = VOCABULARY.getLiteralName(i);
			if (tokenNames[i] == null) {
				tokenNames[i] = VOCABULARY.getSymbolicName(i);
			}

			if (tokenNames[i] == null) {
				tokenNames[i] = "<INVALID>";
			}
		}
	}

	@Override
	@Deprecated
	public String[] getTokenNames() {
		return tokenNames;
	}

	@Override

	public Vocabulary getVocabulary() {
		return VOCABULARY;
	}


	public SPLLexer(CharStream input) {
		super(input);
		_interp = new LexerATNSimulator(this,_ATN,_decisionToDFA,_sharedContextCache);
	}

	@Override
	public String getGrammarFileName() { return "CPP14.g4"; }

	@Override
	public String[] getRuleNames() { return ruleNames; }

	@Override
	public String getSerializedATN() { return _serializedATN; }

	@Override
	public String[] getChannelNames() { return channelNames; }

	@Override
	public String[] getModeNames() { return modeNames; }

	@Override
	public ATN getATN() { return _ATN; }

	public static final String _serializedATN =
		"\3\u608b\ua72a\u8133\ub9ed\u417c\u3be7\u7786\u5964\2\36\u00af\b\1\4\2"+
		"\t\2\4\3\t\3\4\4\t\4\4\5\t\5\4\6\t\6\4\7\t\7\4\b\t\b\4\t\t\t\4\n\t\n\4"+
		"\13\t\13\4\f\t\f\4\r\t\r\4\16\t\16\4\17\t\17\4\20\t\20\4\21\t\21\4\22"+
		"\t\22\4\23\t\23\4\24\t\24\4\25\t\25\4\26\t\26\4\27\t\27\4\30\t\30\4\31"+
		"\t\31\4\32\t\32\4\33\t\33\4\34\t\34\4\35\t\35\3\2\3\2\3\3\3\3\3\3\3\3"+
		"\3\3\3\3\3\3\3\3\3\3\3\4\3\4\3\5\3\5\3\6\3\6\3\7\3\7\3\b\3\b\3\t\3\t\3"+
		"\n\3\n\3\n\3\13\3\13\3\13\3\f\3\f\3\r\3\r\3\16\3\16\3\16\3\17\3\17\3\17"+
		"\3\17\3\20\3\20\3\20\3\20\3\20\3\20\3\21\3\21\3\22\3\22\3\23\3\23\3\23"+
		"\3\23\3\23\3\23\3\24\3\24\3\24\3\25\3\25\3\25\3\25\3\25\3\25\3\25\3\25"+
		"\3\25\3\25\3\26\3\26\3\26\3\26\3\26\3\26\3\26\3\27\3\27\3\30\3\30\3\31"+
		"\3\31\3\31\3\31\3\32\6\32\u0091\n\32\r\32\16\32\u0092\3\33\3\33\7\33\u0097"+
		"\n\33\f\33\16\33\u009a\13\33\6\33\u009c\n\33\r\33\16\33\u009d\3\34\6\34"+
		"\u00a1\n\34\r\34\16\34\u00a2\6\34\u00a5\n\34\r\34\16\34\u00a6\3\35\6\35"+
		"\u00aa\n\35\r\35\16\35\u00ab\3\35\3\35\2\2\36\3\3\5\4\7\5\t\6\13\7\r\b"+
		"\17\t\21\n\23\13\25\f\27\r\31\16\33\17\35\20\37\21!\22#\23%\24\'\25)\26"+
		"+\27-\30/\31\61\32\63\33\65\34\67\359\36\3\2\6\5\2C\\aac|\6\2\62;C\\a"+
		"ac|\3\2\62;\5\2\13\f\17\17\"\"\2\u00b4\2\3\3\2\2\2\2\5\3\2\2\2\2\7\3\2"+
		"\2\2\2\t\3\2\2\2\2\13\3\2\2\2\2\r\3\2\2\2\2\17\3\2\2\2\2\21\3\2\2\2\2"+
		"\23\3\2\2\2\2\25\3\2\2\2\2\27\3\2\2\2\2\31\3\2\2\2\2\33\3\2\2\2\2\35\3"+
		"\2\2\2\2\37\3\2\2\2\2!\3\2\2\2\2#\3\2\2\2\2%\3\2\2\2\2\'\3\2\2\2\2)\3"+
		"\2\2\2\2+\3\2\2\2\2-\3\2\2\2\2/\3\2\2\2\2\61\3\2\2\2\2\63\3\2\2\2\2\65"+
		"\3\2\2\2\2\67\3\2\2\2\29\3\2\2\2\3;\3\2\2\2\5=\3\2\2\2\7F\3\2\2\2\tH\3"+
		"\2\2\2\13J\3\2\2\2\rL\3\2\2\2\17N\3\2\2\2\21P\3\2\2\2\23R\3\2\2\2\25U"+
		"\3\2\2\2\27X\3\2\2\2\31Z\3\2\2\2\33\\\3\2\2\2\35_\3\2\2\2\37c\3\2\2\2"+
		"!i\3\2\2\2#k\3\2\2\2%m\3\2\2\2\'s\3\2\2\2)v\3\2\2\2+\u0080\3\2\2\2-\u0087"+
		"\3\2\2\2/\u0089\3\2\2\2\61\u008b\3\2\2\2\63\u0090\3\2\2\2\65\u009b\3\2"+
		"\2\2\67\u00a4\3\2\2\29\u00a9\3\2\2\2;<\7=\2\2<\4\3\2\2\2=>\7x\2\2>?\7"+
		"c\2\2?@\7t\2\2@A\7k\2\2AB\7c\2\2BC\7d\2\2CD\7g\2\2DE\7n\2\2E\6\3\2\2\2"+
		"FG\7?\2\2G\b\3\2\2\2HI\7/\2\2I\n\3\2\2\2JK\7#\2\2K\f\3\2\2\2LM\7\61\2"+
		"\2M\16\3\2\2\2NO\7,\2\2O\20\3\2\2\2PQ\7-\2\2Q\22\3\2\2\2RS\7>\2\2ST\7"+
		"?\2\2T\24\3\2\2\2UV\7@\2\2VW\7?\2\2W\26\3\2\2\2XY\7>\2\2Y\30\3\2\2\2Z"+
		"[\7@\2\2[\32\3\2\2\2\\]\7#\2\2]^\7?\2\2^\34\3\2\2\2_`\7Q\2\2`a\7E\2\2"+
		"ab\7J\2\2b\36\3\2\2\2cd\7G\2\2de\7N\2\2ef\7N\2\2fg\7G\2\2gh\7T\2\2h \3"+
		"\2\2\2ij\7*\2\2j\"\3\2\2\2kl\7+\2\2l$\3\2\2\2mn\7u\2\2no\7m\2\2op\7t\2"+
		"\2pq\7k\2\2qr\7x\2\2r&\3\2\2\2st\7q\2\2tu\7o\2\2u(\3\2\2\2vw\7c\2\2wx"+
		"\7p\2\2xy\7p\2\2yz\7c\2\2z{\7t\2\2{|\7u\2\2|}\7\"\2\2}~\7q\2\2~\177\7"+
		"o\2\2\177*\3\2\2\2\u0080\u0081\7c\2\2\u0081\u0082\7p\2\2\u0082\u0083\7"+
		"p\2\2\u0083\u0084\7c\2\2\u0084\u0085\7t\2\2\u0085\u0086\7u\2\2\u0086,"+
		"\3\2\2\2\u0087\u0088\7}\2\2\u0088.\3\2\2\2\u0089\u008a\7\177\2\2\u008a"+
		"\60\3\2\2\2\u008b\u008c\7p\2\2\u008c\u008d\7\u00e6\2\2\u008d\u008e\7t"+
		"\2\2\u008e\62\3\2\2\2\u008f\u0091\4c|\2\u0090\u008f\3\2\2\2\u0091\u0092"+
		"\3\2\2\2\u0092\u0090\3\2\2\2\u0092\u0093\3\2\2\2\u0093\64\3\2\2\2\u0094"+
		"\u0098\t\2\2\2\u0095\u0097\t\3\2\2\u0096\u0095\3\2\2\2\u0097\u009a\3\2"+
		"\2\2\u0098\u0096\3\2\2\2\u0098\u0099\3\2\2\2\u0099\u009c\3\2\2\2\u009a"+
		"\u0098\3\2\2\2\u009b\u0094\3\2\2\2\u009c\u009d\3\2\2\2\u009d\u009b\3\2"+
		"\2\2\u009d\u009e\3\2\2\2\u009e\66\3\2\2\2\u009f\u00a1\t\4\2\2\u00a0\u009f"+
		"\3\2\2\2\u00a1\u00a2\3\2\2\2\u00a2\u00a0\3\2\2\2\u00a2\u00a3\3\2\2\2\u00a3"+
		"\u00a5\3\2\2\2\u00a4\u00a0\3\2\2\2\u00a5\u00a6\3\2\2\2\u00a6\u00a4\3\2"+
		"\2\2\u00a6\u00a7\3\2\2\2\u00a78\3\2\2\2\u00a8\u00aa\t\5\2\2\u00a9\u00a8"+
		"\3\2\2\2\u00aa\u00ab\3\2\2\2\u00ab\u00a9\3\2\2\2\u00ab\u00ac\3\2\2\2\u00ac"+
		"\u00ad\3\2\2\2\u00ad\u00ae\b\35\2\2\u00ae:\3\2\2\2\t\2\u0092\u0098\u009d"+
		"\u00a2\u00a6\u00ab\3\b\2\2";
	public static final ATN _ATN =
		new ATNDeserializer().deserialize(_serializedATN.toCharArray());
	static {
		_decisionToDFA = new DFA[_ATN.getNumberOfDecisions()];
		for (int i = 0; i < _ATN.getNumberOfDecisions(); i++) {
			_decisionToDFA[i] = new DFA(_ATN.getDecisionState(i), i);
		}
	}
}