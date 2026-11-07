// Clone Hero v1.0.0.4045 (Mono build), decompiled with ilspycmd 11.0.0.9375 from
// C:\Clone Hero\1.0 old\Clone Hero\Clone Hero_Data\Managed\Assembly-CSharp.dll on 2026-10-30.
// Only the relevant methods are kept, copied unchanged. Names are the game's scrambled names.
// Line numbers in each banner are lines of the full decompiled file, not of this file.

// ---- F6 trigger DrumsPlayer.ʸʸʳʽʲʻʿʸʳʸʽ: SP meter crosses one half : DrumsPlayer.cs lines 870 to 877 of the ilspycmd output ----
	protected override void ʸʸʳʽʲʻʿʸʳʸʽ(float ʴʷˀʳʿʴʿʹʴʾʸ, bool ˀʵʼˁʸʵʻʽʹʻˁ)
	{
		if (ʹˀʼʵʳˀʻʼʵʿʵ < 0.5f && ʹˀʼʵʳˀʻʼʵʿʵ + ʴʷˀʳʿʴʿʹʴʾʸ >= 0.5f && !ˁʸʴʵʳʾʽʿʹʷʿ)
		{
			ʻˀʻʿʷʷʾʿˁʵʽ(ˀʹʳʴʸʺʳʴʷʵʽ.ʲʻʺʹʺʲʷʾʻʺʹ);
		}
		base.ʸʸʳʽʲʻʿʸʳʸʽ(ʴʷˀʳʿʴʿʹʴʾʸ, ˀʵʼˁʸʵʻʽʹʻˁ: true);
	}

// ---- F7 deadline check DrumsPlayer.ʻˀʻʿʷʷʾʿˁʵʽ : DrumsPlayer.cs lines 223 to 237 of the ilspycmd output ----
	public void ʻˀʻʿʷʷʾʿˁʵʽ(double ʿʺʳˁʶʷˁʹʲʺˁ)
	{
		foreach (ʲʺʲʴʶʴʶʻʺʷʴ item in ʲʻʳʸʵʲˀʿʺʴʼ)
		{
			if ((double)item.ʲʸʺʺʼʷʵʵʶʵˁ > ʿʺʳˁʶʷˁʹʲʺˁ + 10.0)
			{
				break;
			}
			if ((double)item.ʲʸʺʺʼʷʵʵʶʵˁ < ʿʺʳˁʶʷˁʹʲʺˁ + (double)(item.ʻʾʺʸʾʹʿʳʴʸʽ * 1f) || (double)item.ʲʸʺʺʼʷʵʵʶʵˁ < ʿʺʳˁʶʷˁʹʲʺˁ + 0.25)
			{
				item.ʵˀʺʾʼʿʷʼʲʲʷ = false;
				item.ʹʾʽʿʻʻʾʿʼʳʸ.ˀʷˁʵʹʳʿʵʼʹˀ &= ~ʼˀˀʸʸʶʳʼʳʸʶ.ʴʴʶʹʼʳʽʵʴʳʿ.DRUM_ACTIVATION;
			}
		}
	}

// ---- F8 base SP gain BasePlayer.ʸʸʳʽʲʻʿʸʳʸʽ : BasePlayer.cs lines 753 to 776 of the ilspycmd output ----
	protected virtual void ʸʸʳʽʲʻʿʸʳʸʽ(float ʴʷˀʳʿʴʿʹʴʾʸ, bool ˀʵʼˁʸʵʻʽʹʻˁ = false)
	{
		if (!ʾʵʻʳʷʴʼʲʻʶʷ.ʲʼʽʺʿʹʽʻʵʺʶ.isRemotePlayer && !ʾʵʻʳʷʴʼʲʻʶʷ.ˀʵʲʲʻʸʸʽʹʽˁ())
		{
			bool flag = ʹˀʼʵʳˀʻʼʵʿʵ < 0.5f && !ˁʸʴʵʳʾʽʿʹʷʿ;
			float num = ʹˀʼʵʳˀʻʼʵʿʵ;
			ʹˀʼʵʳˀʻʼʵʿʵ += ʴʷˀʳʿʴʿʹʴʾʸ;
			if (ˀʵʼˁʸʵʻʽʹʻˁ)
			{
				ʾʻʷʶʹʻʳʸʲʶʹ++;
				ʹˁʾʼʼʹʸʹʿʺʽ.ʾˁʶʵʺʸʽʾˀʸʸ(ʵʻʸʺʸʷʾʿʺˀʴ.StarPowerAwarded);
			}
			else if (flag && ʹˀʼʵʳˀʻʼʵʿʵ >= 0.5f)
			{
				ʹˁʾʼʼʹʸʹʿʺʽ.ʾˁʶʵʺʸʽʾˀʸʸ(ʵʻʸʺʸʷʾʿʺˀʴ.StarPowerAvailable);
			}
			if (ʹˀʼʵʳˀʻʼʵʿʵ >= 1f)
			{
				ʹˀʼʵʳˀʻʼʵʿʵ = 1f;
			}
			ʵʵʴʻʴʾʲʵʵˀʴ += ʹˀʼʵʳˀʻʼʵʿʵ - num;
			ʳʿʿˀʹʻʳʴʳʹʹ.ʸʿʲˁʴʾˁʴʻʳʼ(ʹˀʼʵʳˀʻʼʵʿʵ, ˁʸʴʵʳʾʽʿʹʷʿ);
		}
	}

