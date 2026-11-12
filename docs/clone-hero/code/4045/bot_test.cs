// ---- B1 the bot test, the whole file ʳʼʽʼˀʴʽʵʵʳʵ.cs, lines 1 to 7 of the ilspycmd output ----
public static class ʳʼʽʼˀʴʽʵʵʳʵ
{
	public static bool ˀʵʲʲʻʸʸʽʹʽˁ(this ˁʻʽʷʽʻʾʵʷˀʶ ʼˀʹʲʵʹʶʹʿʿʼ)
	{
		return ʼˀʹʲʵʹʶʹʿʿʼ.ʲʼʽʺʿʹʽʻʵʺʶ.isBot;
	}
}

// ---- B2 the base Star Power gain, BasePlayer.cs lines 753 to 756 (the whole method is F8 in fill_deadline.cs) ----
	protected virtual void ʸʸʳʽʲʻʿʸʳʸʽ(float ʴʷˀʳʿʴʿʹʴʾʸ, bool ˀʵʼˁʸʵʻʽʹʻˁ = false)
	{
		if (!ʾʵʻʳʷʴʼʲʻʶʷ.ʲʼʽʺʿʹʽʻʵʺʶ.isRemotePlayer && !ʾʵʻʳʷʴʼʲʻʶʷ.ˀʵʲʲʻʸʸʽʹʽˁ())
		{

// ---- B3 the command-line start makes a bot profile, CliGameLauncher.cs lines 680 to 685 ----
		ˁʻʽʷʽʻʾʵʷˀʶ obj = GlobalVariables.ʷʲʺʷʻʳʾʶˁˀʷ.ʸʽʲʲʷʵʵʳˀʽʿ[ʸʾʾʴʳʲʿʻʴʾʵ];
		obj.ʾʻʵʷʹʲʶʿʹʷʷ = ʸʾʾʴʳʲʿʻʴʾʵ;
		obj.ʾʵʻʳʷʴʼʲʻʶʷ = ReInput.players.GetSystemPlayer();
		ʹʺʿʻʲʹʻʹʹʵʸ ʹʺʿʻʲʹʻʹʹʵʸ2 = ˀʶˁʻʷˁʻʺʴʸʺ.ʽʲʾʸʿʷʲʶʲʻˁ();
		ʹʺʿʻʲʹʻʹʹʵʸ2.isBot.CurrentValue = 1;

// ---- B4 the meter writes in BasePlayer.cs found by a search for every assignment to the meter field ----
// line 283:  ʹˀʼʵʳˀʻʼʵʿʵ -= ʴʷˀʳʿʴʿʹʴʾʸ;           (the drain while Star Power is active)
// line 533:  ʹˀʼʵʳˀʻʼʵʿʵ = 0f;                     (reset)
// line 759:  ʹˀʼʵʳˀʻʼʵʿʵ += ʴʷˀʳʿʴʿʹʴʾʸ;           (inside B2's test: the only add)
// line 771:  ʹˀʼʵʳˀʻʼʵʿʵ = 1f;                     (cap, inside B2's test)
// line 859:  ʹˀʼʵʳˀʻʼʵʿʵ = ˁˀʸʾʿʶʾʺʷʾʺ.ʳʿʴʵʴʾˀʸʳʶʶ;  (copied back from a saved state; line 596 is the only write of that state field, and it stores this same meter)
